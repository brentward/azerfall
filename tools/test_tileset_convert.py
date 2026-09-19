import tempfile
import re
import subprocess
import sys
import json
import unittest
from pathlib import Path

from PIL import Image

from tileset_convert import (
    convert_tile, convert_tileset, parse_tile_mapping, read_world_map, read_tile_data,
    custom_palette,
)


class MapTilesetTests(unittest.TestCase):
    def test_custom_palette(self):
        image = Image.new("P", (16, 16), 3)
        image.putpalette([0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255])
        image.info["transparency"] = 1
        image.putpixel((0, 0), 1)
        image.putpixel((1, 0), 0)
        indexed, words = custom_palette(image)
        self.assertEqual(indexed.getpixel((2, 0)), 3)
        self.assertEqual(words[3], 0xF820)
        self.assertEqual(words[1] & 0x20, 0)
        self.assertEqual(words[0], 0x20)  # Opaque black is valid at index zero.
        rgba = Image.new("RGBA", (16, 16), (0, 0, 0, 255))
        for i in range(17):
            rgba.putpixel((i % 16, i // 16), (i, 0, 0, 255))
        with self.assertRaisesRegex(ValueError, "WARNING.*17 colors"):
            custom_palette(rgba)

    def test_sheet_cli(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            image = Image.new("RGBA", (16, 32), (255, 0, 0, 255))
            image.paste((0, 0, 0, 0), (0, 16, 16, 32))
            image.save(base / "sheet.png")
            (base / "tiles.txt").write_text("0\nfalse\n1\ntrue\n")
            result = subprocess.run([
                sys.executable, str(Path(__file__).with_name("tileset_convert.py")),
                str(base / "sheet.png"), "--tile-data", str(base / "tiles.txt"),
                "-o", str(base / "out"),
            ], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            source = (base / "out.c").read_text()
            self.assertEqual(source.count("0x00,"), 128)
            self.assertEqual(source.count("0x11,"), 128)
            self.assertIn("0x003F", source)
            metadata = json.loads((base / "out.json").read_text())
            self.assertEqual(metadata["tiles"]["1"]["source_index"], 1)
            self.assertTrue(metadata["tiles"]["1"]["collision"])

    def test_cli_emits_remapped_c_array(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            Image.new("RGBA", (32, 16), (255, 85, 85, 255)).save(base / "tiles.png")
            (base / "map.txt").write_text("9 2 9\n2 9 2\n")
            result = subprocess.run([
                sys.executable, str(Path(__file__).with_name("tileset_convert.py")),
                str(base / "tiles.png"), "--map", str(base / "map.txt"),
                "--mapping", "2=1,9=0", "--name", "test_tiles",
                "-o", str(base / "output"),
            ], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            header = (base / "output.h").read_text()
            self.assertIn("#define TEST_TILES_MAP_WIDTH 3", header)
            self.assertIn("#define TEST_TILES_MAP_HEIGHT 2", header)
            self.assertIn("#define TEST_TILES_MAP_TOTAL_BYTES 6", header)
            self.assertIn("extern const uint8_t test_tiles_map[TEST_TILES_MAP_TOTAL_BYTES];", header)
            source = (base / "output.c").read_text()
            initializer = source.split("const uint8_t test_tiles_map", 1)[1].split("{", 1)[1].split("}", 1)[0]
            self.assertEqual([int(v) for v in re.findall(r"\d+", initializer)], [1, 0, 1, 0, 1, 0])
            self.assertEqual((base / "output.map.txt").read_text(), "1 0 1\n0 1 0\n")

    def test_selection_order_and_blank_preservation(self):
        image = Image.new("RGBA", (48, 16))
        image.paste((255, 85, 85, 255), (0, 0, 16, 16))
        image.paste((85, 255, 85, 255), (32, 0, 48, 16))
        data, _, across, down = convert_tileset(image, [2, 1, 0, 2])
        self.assertEqual((across, down), (3, 1))
        self.assertEqual(data, bytes([0xAA]) * 128 + bytes(128)
                         + bytes([0x99]) * 128 + bytes([0xAA]) * 128)
        legacy, mapping, _, _ = convert_tileset(image)
        self.assertEqual(mapping, {0: 0, 1: None, 2: 1})
        self.assertEqual(len(legacy), 256)
        with self.assertRaises(ValueError):
            convert_tileset(image, [3])

    def test_map_validation(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "map.txt"
            path.write_text("0 5\n\n5 0\n")
            self.assertEqual(read_world_map(path), [[0, 5], [5, 0]])
            for invalid in ("", "0 1\n2", "0 -1", "0 tree"):
                path.write_text(invalid)
                with self.assertRaises(ValueError):
                    read_world_map(path)
        self.assertEqual(parse_tile_mapping("0=0, 5=47"), {0: 0, 5: 47})
        for invalid in ("0=1,0=2", "0=-1", "0:1"):
            with self.assertRaises(ValueError):
                parse_tile_mapping(invalid)

    def test_numbered_tiles_multiple_maps(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            Image.new("RGBA", (16, 16)).save(base / "000.png")
            Image.new("RGBA", (16, 16), (255, 85, 85, 255)).save(base / "001.png")
            (base / "tiledata.txt").write_text("001.png\ntrue\n000.png\nfalse\n")
            (base / "first.txt").write_text("1 0\n")
            (base / "second.txt").write_text("0\n0\n")
            result = subprocess.run([
                sys.executable, str(Path(__file__).with_name("tileset_convert.py")),
                str(base), "--tile-data", str(base / "tiledata.txt"),
                "--map", str(base / "first.txt"), "--map", str(base / "second.txt"),
                "--name", "shared", "-o", str(base / "out"),
            ], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual((base / "out_first_map.txt").read_text(), "1 0\n")
            self.assertEqual((base / "out_second_map.txt").read_text(), "0\n0\n")
            header = (base / "out_first_map.h").read_text()
            self.assertIn("#define SHARED_FIRST_MAP_TOTAL_X (SHARED_FIRST_MAP_WIDTH * SHARED_TILE_WIDTH)", header)
            self.assertIn("#define SHARED_FIRST_MAP_TOTAL_Y (SHARED_FIRST_MAP_HEIGHT * SHARED_TILE_HEIGHT)", header)
            source = (base / "out.c").read_text()
            self.assertIn("shared_collision[SHARED_TILE_COUNT] = {\n    0, 1", source)
            self.assertEqual(source.count("0x00"), 128)
            self.assertEqual(source.count("0x99"), 128)
            metadata = json.loads((base / "out.json").read_text())
            self.assertEqual(metadata["packed_tile_count"], 2)
            self.assertEqual(len(metadata["maps"]), 2)

    def test_tile_data_validation(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "tiledata.txt"
            for invalid in ("", "000.png", "000.png\nmaybe", "../000.png\ntrue",
                            "001.png\nfalse", "000.png\ntrue\n00.png\nfalse"):
                path.write_text(invalid)
                with self.assertRaises(ValueError):
                    read_tile_data(path)

    def test_new_world_source_tiles(self):
        root = Path(__file__).resolve().parents[1]
        rows = read_world_map(root / "assets/maps/worldmap.txt")
        ids = sorted({value for row in rows for value in row})
        entries = read_tile_data(root / "assets/maps/tiledata.txt")
        self.assertTrue(set(ids).issubset(range(len(entries))))
        for filename, _ in entries:
            with Image.open(root / "assets/tiles" / filename) as png:
                self.assertEqual(png.size, (16, 16))


if __name__ == "__main__":
    unittest.main()
