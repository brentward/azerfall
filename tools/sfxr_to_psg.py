"""Convert sfxr.me Base58 exports to 60 Hz RP6502 PSG register frames.

Format/formulas: https://github.com/chr15m/jsfxr/blob/master/sfxr.js
PSG registers: https://picocomputer.github.io/ria.html#programmable-sound-generator
Only Python's standard library is required.
"""
import argparse
import json
import math
from pathlib import Path
import re
import struct

ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
FIELDS = """env_attack env_sustain env_punch env_decay base_freq freq_limit
freq_ramp freq_dramp vib_strength vib_speed arp_mod arp_speed duty duty_ramp
repeat_speed pha_offset pha_ramp lpf_freq lpf_ramp lpf_resonance
hpf_freq hpf_ramp""".split()
SIGNED = set("freq_ramp freq_dramp arp_mod duty_ramp pha_offset pha_ramp lpf_ramp hpf_ramp".split())
AMPLITUDES = [256, 204, 168, 142, 120, 102, 86, 73, 61, 50, 40, 31, 22, 14, 7, 0]
WAVES = [0x10, 0x20, 0x00, 0x40]


def decode(code):
    code = code.strip().removeprefix("#")
    if not code:
        raise ValueError("Empty export")
    n = 0
    for c in code:
        if c not in ALPHABET:
            raise ValueError("Invalid Base58 character: " + c)
        n = n * 58 + ALPHABET.index(c)
    data = bytes(len(code) - len(code.lstrip("1"))) + n.to_bytes((n.bit_length() + 7) // 8, "big")
    if len(data) != 89 or data[0] > 3:
        raise ValueError("Expected a sfxr.me export: one waveform byte and 22 floats")
    params = dict(zip(FIELDS, struct.unpack("<22f", data[1:])))
    for key, value in params.items():
        if not math.isfinite(value) or not (-1 if key in SIGNED else 0) <= value <= 1:
            raise ValueError("Unsupported parameter range: " + key)
    params["wave_type"] = data[0]
    return params


def convert(p, volume=0.7, pan=0):
    if not 0 <= volume <= 1 or not -63 <= pan <= 63:
        raise ValueError("Volume must be 0..1 and pan -63..63")
    warnings = []
    if p["pha_offset"] or p["pha_ramp"]:
        warnings.append("Flanger omitted: PSG has no flanger.")
    if p["lpf_freq"] != 1 or p["lpf_ramp"] or p["lpf_resonance"] or p["hpf_freq"] or p["hpf_ramp"]:
        warnings.append("Filters omitted: PSG has no low/high-pass filter.")
    if p["wave_type"] == 3:
        warnings.append("PSG noise differs from sfxr's 32-sample noise waveform.")
    lengths = [int(p[k] ** 2 * 100000) for k in ("env_attack", "env_sustain", "env_decay")]
    total = sum(lengths)
    repeat = int((1 - p["repeat_speed"]) ** 2 * 20000 + 32) if p["repeat_speed"] else 0
    arp_time = int((1 - p["arp_speed"]) ** 2 * 20000 + 32) if p["arp_speed"] != 1 else 0
    arp_factor = 1 - p["arp_mod"] ** 2 * .9 if p["arp_mod"] >= 0 else 1 + p["arp_mod"] ** 2 * 10
    period_max = 100 / (p["freq_limit"] ** 2 + .001)
    frames = []
    phase = 0
    clipped = False
    # Evolve pitch at sfxr's 44100 Hz control rate, sample at 60 Hz.
    # Envelope is sampled at each frame midpoint so short effects stay audible.
    for t in range(max(1, total)):
        if t == 0 or repeat and t % repeat == 0:
            period = 100 / (p["base_freq"] ** 2 + .001)
            multiplier = 1 - p["freq_ramp"] ** 3 * .01
            duty = .5 - p["duty"] * .5
            arp_pending = bool(arp_time)
        if arp_pending and t >= arp_time:
            period *= arp_factor
            arp_pending = False
        multiplier -= p["freq_dramp"] ** 3 * .000001
        period *= multiplier
        if period > period_max:
            period = period_max
            if p["freq_limit"]:
                break
        phase += p["vib_speed"] ** 2 * .01
        mod_period = period * (1 + math.sin(phase) * p["vib_strength"] * .5)
        hz = 44100 * 8 / max(8, math.floor(mod_period))
        duty = max(0, min(.5, duty - p["duty_ramp"] * .00005))
        if t % 735:
            continue
        at = min(t + 367, max(0, total - 1))
        a, s, d = lengths
        if at < a:
            level = at / a
        elif at < a + s:
            level = 1 + (1 - (at - a) / s) * 2 * p["env_punch"]
        else:
            level = max(0, 1 - (at - a - s) / max(1, d))
        # Normalize punch to retain its shape rather than clipping the peak.
        amplitude = level * volume / (1 + 2 * p["env_punch"])
        attenuation = min(range(16), key=lambda i: abs(AMPLITUDES[i] / 256 - amplitude))
        freq = round(hz * 3)
        clipped |= freq > 65535
        freq = min(65535, freq)
        pwm = round(duty * 255) if p["wave_type"] == 0 else 128
        frames.append([freq & 255, freq >> 8, pwm, attenuation << 4,
                       attenuation << 4, WAVES[p["wave_type"]], (pan * 2 & 254) | 1, 0])
    if not frames:
        frames = [[0, 0, 128, 240, 240, WAVES[p["wave_type"]], 0, 0]]
    if clipped:
        warnings.append("Pitch clipped to PSG maximum (21845 Hz).")
    return frames, warnings


def header(name, frames):
    if not re.fullmatch(r"[A-Za-z][A-Za-z0-9_]*", name):
        raise ValueError("Name must be a C identifier starting with a letter")
    guard = name.upper() + "_H"
    rows = ["    " + ", ".join("0x%02X" % b for b in row) + "," for row in frames]
    return "\n".join([
        "/* Generated by tools/sfxr_to_psg.py; eight PSG bytes per 60 Hz frame. */",
        "#ifndef " + guard, "#define " + guard, "#include <stdint.h>",
        "#define " + name.upper() + "_FRAMES " + str(len(frames)) + "U",
        "static const uint8_t " + name + "[] = {", *rows, "};", "#endif", ""])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--code", help="sfxr.me Base58 export")
    source.add_argument("--input", type=Path, help="Text file containing the export")
    parser.add_argument("--output", type=Path, required=True, help="Generated C header")
    parser.add_argument("--name", default="sfx_example")
    parser.add_argument("--volume", type=float, default=.7, help="Peak amplitude 0..1")
    parser.add_argument("--pan", type=int, default=0, help="-63 left .. 63 right")
    parser.add_argument("--report", type=Path, help="Optional decoded parameters and frames as JSON")
    args = parser.parse_args()
    try:
        params = decode(args.input.read_text() if args.input else args.code)
        frames, warnings = convert(params, args.volume, args.pan)
        output = header(args.name, frames)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(output)
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(json.dumps(dict(parameters=params, frames=frames, warnings=warnings), indent=2) + "\n")
    except (ValueError, OSError) as error:
        parser.error(str(error))
    print(f"{len(frames)} frames, {len(frames) * 8} bytes, {len(frames) / 60:.3f} seconds")
    for warning in warnings:
        print("Approximation: " + warning)


if __name__ == "__main__":
    main()
