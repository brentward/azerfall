"""Verify the current, effect-free RPT3 export and locate sequence 03 row 00.

Run after replacing Z3LIGHTW.RPT/BIN; offsets in game.c must be updated if
the reported positions change. Uses RPTracker's native OPL2 pitch table
and 8.8 fixed-point row clock (see build/RPTracker-reference/src/player.c).
"""
from pathlib import Path
import struct
from fractions import Fraction

from midi_to_rpt import read_midi, extract_notes


def check():
    root = Path(__file__).resolve().parents[1]
    rpt = (root / "assets/music/Z3LIGHTW.RPT").read_bytes()
    stream = (root / "assets/music/Z3LIGHTW.BIN").read_bytes()
    if len(rpt) != 46346 or rpt[:4] != b"RPT3":
        raise ValueError("This checker requires an RPT3 song")
    if len(stream) % 4:
        raise ValueError("BIN has an incomplete packet")
    length, bpm = struct.unpack_from("<HH", rpt, 6)
    if not 3 < length <= 255 or not 1 <= bpm <= 900:
        raise ValueError("Invalid song length or tempo")
    orders = rpt[46090:46090 + length]
    if any(p >= 32 for p in orders):
        raise ValueError("Invalid pattern number")
    ticks = 3600 * 256 // (bpm * 4)
    loop_frame = (96 * ticks + 255) // 256
    fnums = (345, 365, 387, 410, 435, 460, 488, 517, 547, 580, 615, 651)
    expected = []
    for position, pattern in enumerate(orders):
        for row in range(32):
            frame = max(1, ((position * 32 + row) * ticks + 255) // 256)
            for channel in range(9):
                cell = 10 + pattern * 1440 + row * 45 + channel * 5
                note, _, _, effect = struct.unpack_from("<BBBH", rpt, cell)
                if effect:
                    raise ValueError("Effects require a sequencer-aware mapping")
                if 0 < note < 255:
                    if not 12 <= note <= 107:
                        raise ValueError("Note outside supported OPL octaves")
                    fnum = fnums[note % 12]
                    high = 32 | ((note - 12) // 12 << 2) | (fnum >> 8)
                    expected.append((frame, channel, fnum, high))

    actual = []
    registers = [0] * 256
    frame = 0
    loop_offset = None
    last_sounding_frame = 0
    for offset in range(0, len(stream), 4):
        reg, value, delay = struct.unpack_from("<BBH", stream, offset)
        if (reg, value, delay) == (255, 255, 0):
            break
        if frame == loop_frame and loop_offset is None:
            loop_offset = offset
        registers[reg] = value
        if 0xB0 <= reg <= 0xB8 and value & 32:
            fnum = registers[reg - 16] | ((value & 3) << 8)
            actual.append((frame, reg - 0xB0, fnum, value))
        frame += delay
        if any(registers[r] & 32 for r in range(0xB0, 0xB7)):
            last_sounding_frame = frame
    else:
        raise ValueError("BIN has no end marker")
    if actual != expected:
        raise ValueError("BIN note timing/pitches do not match this RPT")
    if loop_offset is None:
        raise ValueError("Target row has no packet; a delay must be split")
    # Musical boundaries must come from the MIDI meter, not the last key-off:
    # another song could finish its last note partway through a measure.
    division, events = read_midi((root / "assets/music/z3lightw.mid").read_bytes())
    meters = {(e.tick, e.data[1], e.data[2]) for e in events
              if e.status == 255 and e.data[0] == 0x58}
    tempos = {int.from_bytes(e.data[1:], "big") for e in events
              if e.status == 255 and e.data[0] == 0x51}
    if meters != {(0, 4, 2)} or len(tempos) != 1:
        raise ValueError("Measure check requires this song's constant 4/4 meter and tempo")
    notes, duration, tempo, _ = extract_notes(division, events)
    source_beats = duration * 1000000 / tempo
    rows_per_beat = 6  # This song's conversion grid; tracker BPM is scaled 6/4.
    if round(Fraction(60000000 * rows_per_beat, tempo * 4)) != bpm:
        raise ValueError("Tracker tempo does not match the six-row MIDI grid")
    if source_beats % 4 or 96 % (4 * rows_per_beat):
        raise ValueError("Loop endpoints are not measure boundaries")
    end_row = source_beats * rows_per_beat
    if end_row.denominator != 1 or frame != (int(end_row) * ticks + 255) // 256:
        raise ValueError("BIN end does not match the MIDI's final measure boundary")
    row_seconds = Fraction(ticks, 256 * 60)
    onset_error = max(abs(int(n.start / row_seconds + Fraction(1, 2)) * row_seconds - n.start)
                      for n in notes)
    print(f"4/4 measure grid: start measure 5, end before measure {int(source_beats / 4) + 1}")
    print(f"Loop length: {int(source_beats / 4) - 4} complete measures")
    print(f"Maximum MIDI-to-tracker note-on quantization: {float(onset_error * 1000):.2f} ms")
    print(f"Matched all {len(actual)} note-on events")
    print(f"Sequence 03 row 00: byte {loop_offset}, frame {loop_frame} ({loop_frame / 60:.3f}s)")
    print(f"End marker: byte {offset}, frame {frame} ({frame / 60:.3f}s)")
    print(f"Audition start (five seconds before end): frame {max(0, frame - 300)}")
    print(f"Time after last key-off: {(frame - last_sounding_frame) / 60:.3f}s (includes release tails)")
    if frame != last_sounding_frame:
        raise ValueError("BIN has trailing delay after the final music key-off; trim it before rebuilding")
    # The final note-offs must go directly into the loop marker. The exporter
    # normally wipes instrument registers here, which can click on release.
    if offset < 4 or not 0xB0 <= stream[offset - 4] <= 0xB6 or stream[offset - 3] & 32:
        raise ValueError("Remove the exporter reset after the final music key-offs")
    # Catch stale integration constants as part of the check.
    game = (root / "src/game/game.c").read_text()
    if f"music_set_loop_offset(&game, {loop_offset}U)" not in game:
        raise ValueError("Update game.c's loop offset to the value above")
    if f"music_skip_to_frame(&game, {max(0, frame - 300)}U)" not in game:
        raise ValueError("Update game.c's audition frame to the value above")


if __name__ == "__main__":
    check()
