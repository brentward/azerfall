#!/usr/bin/env python3
"""Convert Standard MIDI files to RPTracker RPT2/RPT3 (standard library only)."""

import argparse
from collections import Counter, defaultdict, deque
from dataclasses import dataclass
from fractions import Fraction
from pathlib import Path
import struct
import sys
import json
import math


@dataclass
class Event:
    tick: int
    track: int
    index: int
    status: int
    data: bytes


class Reader:
    def __init__(self, data):
        self.data = data
        self.pos = 0

    def take(self, size):
        if self.pos + size > len(self.data):
            raise ValueError("Truncated MIDI file")
        result = self.data[self.pos:self.pos + size]
        self.pos += size
        return result

    def byte(self):
        return self.take(1)[0]

    def vlq(self):
        result = 0
        for _ in range(4):
            value = self.byte()
            result = (result << 7) | (value & 127)
            if not value & 128:
                return result
        raise ValueError("MIDI variable-length value exceeds four bytes")


def read_midi(data):
    reader = Reader(data)
    if reader.take(4) != b"MThd":
        raise ValueError("Expected a Standard MIDI file (MThd)")
    header = reader.take(int.from_bytes(reader.take(4), "big"))
    if len(header) < 6:
        raise ValueError("Invalid MIDI header")
    fmt, tracks, division = struct.unpack(">HHH", header[:6])
    if fmt not in (0, 1) or not tracks or (fmt == 0 and tracks != 1):
        raise ValueError("Only MIDI formats 0 and 1 are supported")
    if not division or division & 0x8000:
        raise ValueError("MIDI must use ticks per quarter note, not SMPTE timing")
    events = []
    for track in range(tracks):
        if reader.take(4) != b"MTrk":
            raise ValueError("Expected MIDI track chunk (MTrk)")
        chunk = Reader(reader.take(int.from_bytes(reader.take(4), "big")))
        tick = index = running = 0
        while chunk.pos < len(chunk.data):
            tick += chunk.vlq()
            status = chunk.byte()
            if status < 128:
                if not running:
                    raise ValueError("MIDI running status has no previous channel status")
                chunk.pos -= 1
                status = running
            if status == 0xFF:
                kind = chunk.byte()
                payload = chunk.take(chunk.vlq())
                events.append(Event(tick, track, index, status, bytes([kind]) + payload))
                running = 0
                if kind == 0x2F:
                    if payload:
                        raise ValueError("Invalid MIDI end-of-track event")
                    break
            elif status in (0xF0, 0xF7):
                events.append(Event(tick, track, index, status, chunk.take(chunk.vlq())))
                running = 0
            elif 0x80 <= status <= 0xEF:
                running = status
                payload = chunk.take(1 if status >> 4 in (0xC, 0xD) else 2)
                if any(value >= 128 for value in payload):
                    raise ValueError("Invalid MIDI channel data byte")
                events.append(Event(tick, track, index, status, payload))
            else:
                raise ValueError(f"Unsupported MIDI status 0x{status:02X}")
            index += 1
    return division, sorted(events, key=lambda e: (e.tick, e.track, e.index))


@dataclass
class Note:
    start: Fraction
    end: Fraction
    channel: int
    pitch: int
    program: int
    volume: int


def extract_notes(division, events):
    """Merge tracks, apply tempo map and sustain, and capture note-on dynamics."""
    programs = [0] * 16
    volumes = [100] * 16
    expression = [127] * 16
    sustain = [False] * 16
    held = defaultdict(deque)
    released = [[] for _ in range(16)]
    notes = []
    warnings = Counter()
    tempo = 500000
    initial_tempo = tempo
    time = Fraction(0)
    last_tick = 0

    def finish(note):
        note.end = time
        notes.append(note)

    for event in events:
        time += Fraction((event.tick - last_tick) * tempo, division * 1000000)
        last_tick = event.tick
        status, data = event.status, event.data
        if status == 0xFF:
            if data[0] == 0x51:
                if len(data) != 4 or int.from_bytes(data[1:], "big") == 0:
                    raise ValueError("Invalid MIDI tempo event")
                tempo = int.from_bytes(data[1:], "big")
                if event.tick == 0:
                    initial_tempo = tempo
            continue
        if status >= 0xF0:
            warnings["System-exclusive messages ignored"] += 1
            continue
        channel, kind = status & 15, status >> 4
        if kind == 0xC:
            programs[channel] = data[0]
        elif kind == 0x9 and data[1]:
            level = data[1] * volumes[channel] * expression[channel]
            volume = max(1, round(Fraction(level * 63, 127**3))) if level else 0
            held[channel, data[0]].append(Note(time, time, channel, data[0], programs[channel], volume))
        elif kind == 0x8 or (kind == 0x9 and not data[1]):
            queue = held[channel, data[0]]
            if queue:
                note = queue.popleft()
                if sustain[channel]:
                    released[channel].append(note)
                else:
                    finish(note)
            else:
                warnings["Unmatched note-offs ignored"] += 1
        elif kind == 0xB:
            controller, value = data
            if controller in (7, 11):
                (volumes if controller == 7 else expression)[channel] = value
                if any(queue for (ch, _), queue in held.items() if ch == channel) or released[channel]:
                    warnings["Volume/expression changes during held notes apply to subsequent notes only"] += 1
            elif controller == 64:
                sustain[channel] = value >= 64
                if not sustain[channel]:
                    for note in released[channel]:
                        finish(note)
                    released[channel].clear()
            elif controller in (120, 123):
                for (ch, _), queue in held.items():
                    if ch == channel:
                        while queue:
                            note = queue.popleft()
                            if controller == 123 and sustain[channel]:
                                released[channel].append(note)
                            else:
                                finish(note)
                if controller == 120:
                    for note in released[channel]:
                        finish(note)
                    released[channel].clear()
            else:
                warnings[f"Controller CC{controller} ignored"] += 1
        elif kind in (0xA, 0xD, 0xE):
            warnings["Pitch bend/aftertouch ignored"] += 1
    for queue in list(held.values()) + released:
        for note in queue:
            finish(note)
            warnings["Unreleased notes ended at MIDI end"] += 1
    return sorted(notes, key=lambda n: (n.start, n.channel, n.pitch)), time, initial_tempo, warnings


ROWS = 32
VOICES = 9
PATTERNS = 32
CELL_SIZE = 5
PATTERN_SIZE = ROWS * VOICES * CELL_SIZE


def convert(data, rows_per_beat=4, bpm=None, overflow="error", rpt_version=3):
    if rpt_version not in (2, 3):
        raise ValueError("RPT version must be 2 or 3")
    division, events = read_midi(data)
    notes, duration, tempo, warnings = extract_notes(division, events)
    if not notes:
        raise ValueError("MIDI contains no notes")
    if rows_per_beat <= 0:
        raise ValueError("Rows per beat must be positive")
    if overflow not in ("error", "drop"):
        raise ValueError("Unknown voice overflow policy")
    # RPTracker always has four rows per beat. Scaling its BPM gives a finer
    # grid without changing playback speed. Flatten MIDI tempo changes to time.
    if bpm is None:
        bpm = round(Fraction(60000000 * rows_per_beat, tempo * 4))
    if not 60 <= bpm <= 240:
        raise ValueError(f"Tracker BPM {bpm} is outside 60..240; choose fewer rows per beat or --bpm")
    # Mirror bpm_to_ticks_fp() and the 60 Hz sequencer, including truncation.
    row_seconds = Fraction(230400 // bpm, 256 * 60)
    scheduled = []
    max_error = Fraction(0)
    for index, note in enumerate(notes):
        start = int(note.start / row_seconds + Fraction(1, 2))
        end = max(start + 1, int(note.end / row_seconds + Fraction(1, 2)))
        max_error = max(max_error, abs(start * row_seconds - note.start), abs(end * row_seconds - note.end))
        if note.channel == 9:
            # GM percussion 35..81 corresponds to RPTracker bank 169..215.
            # Its USB pad mapping is different and must not be used for SMF.
            if not 35 <= note.pitch <= 81:
                raise ValueError(f"Unsupported GM percussion note {note.pitch} (expected 35..81)")
            instrument, pitch = note.pitch + 134, 60
        else:
            instrument, pitch = note.program, note.pitch
            # OPL's eight blocks faithfully represent MIDI notes 12..107.
            if not 12 <= pitch <= 107:
                raise ValueError(f"MIDI note {pitch} is outside RPTracker's faithful OPL pitch range 12..107")
        scheduled.append((start, end, index, note.channel, pitch, instrument, note.volume))
    total_rows = max(math.ceil(duration / row_seconds), max(item[1] for item in scheduled) + 1)
    order_count = math.ceil(total_rows / ROWS)
    # Current tracker increments a uint8_t order index; 255 avoids its 256 wrap bug.
    if order_count > 255:
        raise ValueError(f"Song needs {order_count} sequence entries; maximum is 255")
    grid = bytearray(order_count * PATTERN_SIZE)
    voices = [None] * VOICES
    previous_channels = [None] * VOICES
    dropped = peak = 0

    def cell(row, voice, pitch, instrument=0, volume=0):
        offset = (row * VOICES + voice) * CELL_SIZE
        grid[offset:offset + CELL_SIZE] = bytes((pitch, instrument, volume, 0, 0))

    for start, end, index, channel, pitch, instrument, volume in sorted(scheduled):
        for voice, active in enumerate(voices):
            if active is not None and active[0] <= start:
                cell(active[0], voice, 255)
                voices[voice] = None
        free = [voice for voice, active in enumerate(voices) if active is None]
        if not free:
            if overflow == "error":
                raise ValueError(f"More than nine voices at row {start} ({float(start * row_seconds):.3f}s); "
                                 "use a finer grid or explicitly allow --overflow drop")
            dropped += 1
            continue
        # Stable channel ownership makes patterns readable and repeatable.
        voice = min(free, key=lambda v: (previous_channels[v] != channel,
                                        previous_channels[v] is not None, v))
        # A new note on a release row replaces note-off: the tracker retriggers.
        cell(start, voice, pitch, instrument, volume)
        voices[voice] = (end, index)
        previous_channels[voice] = channel
        peak = max(peak, sum(active is not None for active in voices))
    for voice, active in enumerate(voices):
        if active is not None:
            cell(active[0], voice, 255)
    patterns = []
    order = []
    lookup = {}
    for offset in range(0, len(grid), PATTERN_SIZE):
        pattern = bytes(grid[offset:offset + PATTERN_SIZE])
        if pattern not in lookup:
            lookup[pattern] = len(patterns)
            patterns.append(pattern)
        order.append(lookup[pattern])
    if len(patterns) > PATTERNS:
        raise ValueError(f"Song needs {len(patterns)} unique patterns; RPTracker supports 32. "
                         "Choose fewer rows per beat or a lower --bpm")
    header = f"RPT{rpt_version}".encode("ascii") + struct.pack("<BBH", 3, 63, len(order))
    if rpt_version == 3:
        header += struct.pack("<H", bpm)
    result = (header
              + b"".join(patterns) + bytes((PATTERNS - len(patterns)) * PATTERN_SIZE)
              + bytes(order) + bytes(256 - len(order)))
    report = {
        "format": f"RPT{rpt_version}", "input_notes": len(notes), "written_notes": len(notes) - dropped,
        "dropped_notes": dropped, "peak_voices": peak, "midi_seconds": float(duration),
        "tracker_seconds": float(len(order) * ROWS * row_seconds), "tracker_bpm": bpm,
        "row_milliseconds": float(row_seconds * 1000),
        "maximum_note_timing_error_ms": float(max_error * 1000),
        "unique_patterns": len(patterns), "sequence_entries": len(order),
        "bytes": len(result), "warnings": dict(warnings),
    }
    return result, report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path, nargs="?")
    parser.add_argument("--rows-per-beat", type=int, default=4,
                        help="Grid density at the initial MIDI tempo (default: 4; use 6 for triplets)")
    parser.add_argument("--bpm", type=int, help="Override tracker grid BPM, 60..240; preserves MIDI duration")
    parser.add_argument("--overflow", choices=("error", "drop"), default="error",
                        help="Policy when all nine voices are busy (default: error)")
    parser.add_argument("--report", type=Path, help="Write conversion statistics as JSON")
    parser.add_argument("--rpt-version", type=int, choices=(2, 3), default=3,
                        help="3 (default): embeds tempo for current source builds; 2: patched v0.8, manual tempo")
    args = parser.parse_args()
    try:
        output = args.output or args.input.with_suffix(".rpt")
        paths = [args.input.resolve(), output.resolve()]
        if args.report:
            paths.append(args.report.resolve())
        if len(set(paths)) != len(paths):
            raise ValueError("Input, output, and report must be different files")
        result, report = convert(args.input.read_bytes(), args.rows_per_beat, args.bpm, args.overflow, args.rpt_version)
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_bytes(result)
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(f"Wrote {output}: {report['written_notes']} notes, {report['unique_patterns']} unique patterns, "
              f"{report['sequence_entries']} sequence entries, {report['tracker_bpm']} tracker BPM")
        print(f"Duration: MIDI {report['midi_seconds']:.3f}s, tracker {report['tracker_seconds']:.3f}s; "
              f"maximum note timing error {report['maximum_note_timing_error_ms']:.1f}ms")
        for warning, count in report["warnings"].items():
            print(f"warning: {warning} ({count})", file=sys.stderr)
        if report["dropped_notes"]:
            print(f"warning: Dropped {report['dropped_notes']} notes", file=sys.stderr)
        print("\nWARNING: RPTracker settings before BIN export:", file=sys.stderr)
        if args.rpt_version == 2:
            print("  Unpatched RPTracker v0.8 truncates pattern transfers at 32767 bytes.\n"
                  "  Use the load/save fix in tools/rptracker-v08-io.patch or a fixed newer build.",
                  file=sys.stderr)
            print(f"  Set tempo to {report['tracker_bpm']} BPM using F7 (increase) / Shift+F7 (decrease).\n"
                  "  RPT2 does not save tempo; check it each time you load the song.", file=sys.stderr)
        else:
            print(f"  Verify tempo is {report['tracker_bpm']} BPM (stored in RPT3).\n"
                  "  RPT3 requires a newer loader; the v0.8 release cannot load it correctly.", file=sys.stderr)
        print("  Select SONG mode with F8 and audition the full song with Enter.\n"
              "  Use a tracker build targeting NATIVE OPL2 for Azerfall.\n"
              "  Press Ctrl+E to export BIN; check the console for export-size-limit warnings.",
              file=sys.stderr)
    except (OSError, ValueError) as error:
        parser.exit(1, f"error: {error}\n")


if __name__ == "__main__":
    main()
