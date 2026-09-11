import tempfile
import re
import subprocess
import sys
import unittest
from pathlib import Path

from PIL import Image

from tileset_convert import (
    convert_tile, convert_tileset, parse_tile_mapping, read_world_map,
)


class MapTilesetTests(unittest.TestCase):
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

    def test_world01_source_tiles(self):
        root = Path(__file__).resolve().parents[1]
        rows = read_world_map(root / "assets/maps/world01.txt")
        ids = sorted({value for row in rows for value in row})
        self.assertEqual(ids, list(range(6)))
        sources = [0, 716, 320, 35, 197, 47]
        with Image.open(root / "assets/tiles/punyworld-overworld-tileset.png") as png:
            image = png.convert("RGBA")
        data, _, across, _ = convert_tileset(image, sources)
        self.assertEqual(len(data), 768)
        for index, source in enumerate(sources):
            expected = convert_tile(image, (source % across) * 16,
                                    (source // across) * 16)
            self.assertEqual(data[index * 128:(index + 1) * 128], expected)


if __name__ == "__main__":
    unittest.main()
