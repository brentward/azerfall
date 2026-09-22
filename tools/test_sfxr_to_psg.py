import math
from pathlib import Path
import unittest

from sfxr_to_psg import decode, convert, header, FIELDS

ROOT = Path(__file__).resolve().parents[1]


class ConversionTests(unittest.TestCase):
    def params(self, **overrides):
        p = dict.fromkeys(FIELDS, 0.0)
        p.update(wave_type=0, base_freq=.3, env_sustain=.3, lpf_freq=1)
        p.update(overrides)
        return p

    def test_supplied_export(self):
        p = decode((ROOT / "assets/sounds/example.sfxr.txt").read_text())
        self.assertEqual(p["wave_type"], 3)
        self.assertAlmostEqual(p["base_freq"], .0250626877)
        frames, warnings = convert(p)
        self.assertEqual(len(frames), 15)
        self.assertTrue(all(len(frame) == 8 for frame in frames))
        self.assertTrue(all(frame[5] == 0x40 for frame in frames))
        self.assertTrue(any("Flanger" in w for w in warnings))
        self.assertEqual(header("sfx_example", frames),
                         (ROOT / "generated/sfx_example.h").read_text())

    def test_constant_pitch_and_envelope_duration(self):
        frames, _ = convert(self.params())
        # 100000 * .3^2 / 44100 seconds, rounded up to 60 Hz frames.
        self.assertEqual(len(frames), math.ceil(9000 / 735))
        expected = round(352800 / math.floor(100 / (.3 ** 2 + .001)) * 3)
        self.assertTrue(all(row[0] + 256 * row[1] == expected for row in frames))
        self.assertTrue(all(row[2] == 128 for row in frames))

    def test_rising_and_falling_pitch(self):
        def frequencies(ramp):
            rows, _ = convert(self.params(freq_ramp=ramp))
            return [row[0] + 256 * row[1] for row in rows]
        up, down = frequencies(.2), frequencies(-.2)
        self.assertGreater(up[-1], up[0])
        self.assertLess(down[-1], down[0])

    def test_register_encoding_and_silence(self):
        rows, _ = convert(self.params(wave_type=2), volume=0, pan=-63)
        self.assertTrue(all(r[3:7] == [240, 240, 0, 131] for r in rows))
        rows, _ = convert(self.params(wave_type=1), pan=63)
        self.assertEqual(rows[0][5:7], [32, 127])

    def test_bad_input(self):
        for code in ("", "0", "abc"):
            with self.assertRaises(ValueError):
                decode(code)
        with self.assertRaises(ValueError):
            convert(self.params(), pan=64)
        with self.assertRaises(ValueError):
            header("bad-name", [])

    def test_square_leading_zero_byte(self):
        # Base58 zero bytes are represented by leading '1' characters.
        with self.assertRaisesRegex(ValueError, "89|Expected"):
            decode("1" * 88)
        p = decode("1" * 89)
        self.assertEqual(p["wave_type"], 0)
        self.assertEqual(p["base_freq"], 0)


if __name__ == "__main__":
    unittest.main()
