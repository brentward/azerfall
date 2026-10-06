"""Behavior and binary-layout checks; run with unittest discovery."""

from fractions import Fraction
from pathlib import Path
import struct
import unittest

from midi_to_rpt import convert, extract_notes, read_midi


def vlq(value):
    result = [value & 127]
    while value >> 7:
        value >>= 7
        result.insert(0, (value & 127) | 128)
    return bytes(result)


def midi(*tracks, ppqn=120):
    result = b"MThd" + struct.pack(">IHHH", 6, int(len(tracks) > 1), len(tracks), ppqn)
    for events in tracks:
        chunk = b"".join(vlq(delta) + message for delta, message in events)
        chunk += b"\x00\xff\x2f\x00"
        result += b"MTrk" + struct.pack(">I", len(chunk)) + chunk
    return result


def decode(blob):
    """Independently read the offsets consumed by RPTracker's load_song()."""
    assert blob[:4] in (b"RPT2", b"RPT3")
    header_size = 8 if blob[:4] == b"RPT2" else 10
    assert len(blob) == header_size + 46080 + 256
    octave, volume, length = struct.unpack_from("<BBH", blob, 4)
    assert (octave, volume) == (3, 63)
    assert 1 <= length <= 255
    if header_size == 10:
        assert 60 <= struct.unpack_from("<H", blob, 8)[0] <= 240
    order = blob[header_size + 46080:header_size + 46080 + length]
    rows = []
    for pattern in order:
        assert pattern < 32
        for row in range(32):
            offset = header_size + (pattern * 32 + row) * 45
            rows.append([struct.unpack_from("<BBBH", blob, offset + channel * 5)
                         for channel in range(9)])
    return rows


class MidiTests(unittest.TestCase):
    def test_v08_loader_compatibility_and_optional_new_format(self):
        source = midi([(0, b"\x90\x3c\x7f"), (120, b"\x80\x3c\x00")])
        legacy, report = convert(source, rpt_version=2)
        modern, _ = convert(source)
        self.assertEqual(modern[:4], b"RPT3")
        self.assertEqual(legacy[:4], b"RPT2")
        self.assertEqual(len(legacy), 46344)
        # v0.8 load_song unconditionally starts XRAM data at byte 8;
        # it accepts RPT3's magic but reads its BPM as part of the first cell.
        self.assertEqual(tuple(legacy[8:13]), (60, 0, 50, 0, 0))
        self.assertNotEqual(tuple(modern[8:13]), (60, 0, 50, 0, 0))
        self.assertEqual(legacy[8:], modern[10:])
        self.assertEqual(struct.unpack_from("<H", modern, 8)[0], report["tracker_bpm"])
        self.assertEqual(decode(legacy), decode(modern))

    def test_running_status_and_velocity_zero_note_off(self):
        source = midi([(0, b"\x90\x3c\x7f"), (120, b"\x3c\x00")])
        notes, duration, _, _ = extract_notes(*read_midi(source))
        self.assertEqual(len(notes), 1)
        self.assertEqual(notes[0].end, Fraction(1, 2))
        self.assertEqual(duration, Fraction(1, 2))

    def test_tempo_map_across_tracks(self):
        source = midi([(0, b"\xff\x51\x03\x07\xa1\x20"),
                       (120, b"\xff\x51\x03\x0f\x42\x40")],
                      [(0, b"\x90\x3c\x7f"), (240, b"\x80\x3c\x00")])
        notes, _, _, _ = extract_notes(*read_midi(source))
        self.assertEqual(notes[0].end, Fraction(3, 2))
        rows = decode(convert(source)[0])
        self.assertEqual(rows[12][0][0], 255)

    def test_sustain_and_program(self):
        source = midi([(0, b"\xc0\x3d"), (0, b"\xb0\x40\x7f"),
                       (0, b"\x90\x3c\x7f"), (120, b"\x80\x3c\x00"),
                       (120, b"\xb0\x40\x00")])
        notes, _, _, _ = extract_notes(*read_midi(source))
        self.assertEqual(notes[0].end, 1)
        self.assertEqual(notes[0].program, 61)
        rows = decode(convert(source)[0])
        self.assertEqual(rows[0][0][1], 61)
        self.assertEqual(rows[8][0][0], 255)

    def test_retrigger_replaces_release_without_killing_next_note(self):
        source = midi([(0, b"\x90\x3c\x7f"), (120, b"\x80\x3c\x00"),
                       (0, b"\x90\x3e\x7f"), (120, b"\x80\x3e\x00")])
        rows = decode(convert(source)[0])
        self.assertEqual([rows[r][0][0] for r in (0, 4, 8)], [60, 62, 255])

    def test_percussion_uses_gm_bank_not_usb_pad_mapping(self):
        source = midi([(0, b"\x99\x26\x7f"), (120, b"\x89\x26\x00"),
                       (0, b"\x99\x31\x7f"), (120, b"\x89\x31\x00")])
        rows = decode(convert(source)[0])
        self.assertEqual(rows[0][0][:2], (60, 172))
        self.assertEqual(rows[4][0][:2], (60, 183))

    def test_zero_channel_volume_remains_silent(self):
        source = midi([(0, b"\xb0\x07\x00"), (0, b"\x90\x3c\x7f"),
                       (120, b"\x80\x3c\x00")])
        self.assertEqual(decode(convert(source)[0])[0][0][2], 0)

    def test_overflow_fails_unless_explicitly_allowed(self):
        events = [(0, bytes((0x90, pitch, 127))) for pitch in range(60, 68)]
        events += [(120 if pitch == 60 else 0, bytes((0x80, pitch, 0))) for pitch in range(60, 68)]
        source = midi(events)
        with self.assertRaisesRegex(ValueError, "More than 7 music voices"):
            convert(source)
        blob, report = convert(source, overflow="drop")
        self.assertEqual(report["dropped_notes"], 1)
        self.assertEqual(sum(cell[0] not in (0, 255) for row in decode(blob) for cell in row), 7)
        self.assertTrue(all(cell == (0, 0, 0, 0) for row in decode(blob) for cell in row[7:]))

    def test_pattern_boundary_release(self):
        source = midi([(0, b"\x90\x3c\x7f"), (960, b"\x80\x3c\x00")])
        rows = decode(convert(source)[0])
        self.assertEqual(rows[32][0][0], 255)

    def test_repeated_patterns_are_reused(self):
        events = []
        for index in range(40):
            events.extend([(0 if index == 0 else 840, b"\x90\x3c\x7f"),
                           (120, b"\x80\x3c\x00")])
        blob, report = convert(midi(events))
        self.assertEqual(report["sequence_entries"], 40)
        self.assertEqual(report["unique_patterns"], 1)
        self.assertEqual(sum(cell[0] == 60 for row in decode(blob) for cell in row), 40)

    def test_pattern_and_sequence_capacity_errors(self):
        events = []
        for index in range(33):
            pitch = 40 + index
            events.extend([(0 if index == 0 else 840, bytes((0x90, pitch, 127))),
                           (120, bytes((0x80, pitch, 0)))])
        with self.assertRaisesRegex(ValueError, "33 unique patterns"):
            convert(midi(events))
        with self.assertRaisesRegex(ValueError, "sequence entries"):
            convert(midi([(0, b"\x90\x3c\x7f"), (960 * 256, b"\x80\x3c\x00")]))

    def test_invalid_midi_and_unsupported_timing(self):
        source = midi([(0, b"\x90\x3c\x7f"), (120, b"\x80\x3c\x00")])
        for bad in (b"", source[:-1], source[:12] + b"\xe7\x28" + source[14:],
                    midi([(0, b"\x3c\x7f")])):
            with self.subTest(bad=bad):
                with self.assertRaises(ValueError):
                    convert(bad)
        with self.assertRaisesRegex(ValueError, "outside 60..240"):
            convert(source, bpm=241)

    def test_project_song_round_trip_layout_and_all_notes(self):
        source = (Path(__file__).resolve().parents[1] / "assets/music/z3lightw.mid").read_bytes()
        blob, report = convert(source, rows_per_beat=6)
        rows = decode(blob)
        self.assertEqual(report["input_notes"], 1270)
        self.assertEqual(report["dropped_notes"], 0)
        self.assertEqual(report["peak_voices"], 7)
        self.assertEqual(report["music_channels"], list(range(7)))
        self.assertEqual(report["reserved_sfx_channels"], [7, 8])
        # Check every stored pattern, including unused patterns, in both formats.
        for version, header_size in ((2, 8), (3, 10)):
            stored, _ = convert(source, rows_per_beat=6, rpt_version=version)
            for offset in range(header_size, header_size + 46080, 45):
                self.assertEqual(stored[offset + 35:offset + 45], bytes(10))
        self.assertEqual(sum(cell[0] not in (0, 255) for row in rows for cell in row), 1270)
        self.assertEqual(report["tracker_bpm"], 204)
        self.assertLessEqual(report["unique_patterns"], 32)
        self.assertEqual(blob, convert(source, rows_per_beat=6)[0])
        active = [False] * 9
        for row in rows:
            for voice, (pitch, instrument, volume, effect) in enumerate(row):
                self.assertLessEqual(volume, 63)
                self.assertEqual(effect, 0)
                if pitch:
                    active[voice] = pitch != 255
        self.assertFalse(any(active), "All voices must release before looping")


if __name__ == "__main__":
    unittest.main()
