import json
import tempfile
import unittest
import subprocess
import sys
from pathlib import Path

from PIL import Image
from sprite_convert import prepare_palette, extract_sprites


class SpritePaletteTests(unittest.TestCase):
    def test_layout_cli(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            image = Image.new("RGBA", (48, 32))
            image.paste((255, 0, 0, 255), (32, 0, 48, 16))
            image.save(base / "sheet.png")
            command = [sys.executable, str(Path(__file__).with_name("sprite_convert.py")),
                       str(base / "sheet.png"), "--name", "test", "--prefix", "test",
                       "-o", str(base / "out")]
            result = subprocess.run(command + ["--layout", "objects"], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            header = (base / "out.h").read_text()
            self.assertIn("#define TEST_IMAGE2 0", header)
            self.assertIn("#define TEST_COUNT 1", header)
            self.assertNotIn("_DIR", header)
            result = subprocess.run(command + ["--layout", "animation", "--frames", "2"],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            header = (base / "out.h").read_text()
            self.assertIn("#define TEST_COUNT 4", header)
            self.assertIn("#define TEST_DIR1_FRAME0 2", header)
            self.assertIn("#define TEST_FRAMES_PER_DIRECTION 2", header)
            self.assertNotIn("FRAME2", header)
            manifest = json.loads((base / "out.json").read_text())
            self.assertEqual([s["source_cell"] for s in manifest["sprites"]], [0, 1, 3, 4])
            result = subprocess.run(command + ["--frames", "4"], capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)

    def test_explicit_world_palette(self):
        root = Path(__file__).resolve().parents[1]
        image = Image.new("RGBA", (16, 16), (0, 0, 0, 255))
        image.putpixel((1, 0), (255, 255, 255, 255))
        lookup, words = prepare_palette(image, palette_path=root / "assets/palettes/overworld.gpl")
        data, _, _, _ = extract_sprites(image, None, None, False, lookup)
        self.assertEqual(data[0], 0x0F)
        self.assertEqual(words[0], 0)
        self.assertEqual(words[15], 0xFFFF)
        lookup, shared = prepare_palette(image, shared_path=root / "generated/world_tiles.json")
        self.assertEqual(lookup[(0, 0, 0, 255)], 0)
        self.assertEqual(lookup[(255, 255, 255, 255)], 15)
        self.assertEqual(words, shared)

    def test_shared_indexes_and_transparency(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "world.json"
            words = [0x20] * 16
            words[7] = 0x3F  # red
            words[13] = 0  # transparent, not index zero
            path.write_text(json.dumps({"palette": words,
                "palette_format": "RP6502 RGB555, alpha bit 5"}))
            image = Image.new("RGBA", (32, 16), (255, 0, 0, 255))
            image.putpixel((0, 0), (0, 0, 0, 0))
            lookup, palette = prepare_palette(image, shared_path=path)
            data, sprites, _, _ = extract_sprites(image, None, None, False, lookup)
            self.assertEqual(data[0], 0xD7)
            self.assertEqual(data[128:], bytes([0x77]) * 128)
            self.assertEqual(len(sprites), 2)
            self.assertEqual(palette, words)
            image.putpixel((0, 0), (0, 255, 0, 255))
            with self.assertRaisesRegex(ValueError, "absent"):
                prepare_palette(image, shared_path=path)

    def test_custom_and_color_key(self):
        image = Image.new("RGBA", (16, 16), (255, 0, 0, 255))
        image.putpixel((0, 0), (0, 0, 0, 255))
        lookup, words = prepare_palette(image, (0, 0, 0))
        data, _, _, _ = extract_sprites(image, None, (0, 0, 0), False, lookup)
        self.assertFalse(words[data[0] >> 4] & 0x20)
        self.assertEqual(words[data[0] & 15], 0x3F)
        for i in range(17):
            image.putpixel((i % 16, i // 16), (i, 0, 0, 255))
        with self.assertRaisesRegex(ValueError, "WARNING"):
            prepare_palette(image)


if __name__ == "__main__":
    unittest.main()
