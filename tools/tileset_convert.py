from PIL import Image
from pathlib import Path
import argparse
import json
import re


TILE_W = 16
TILE_H = 16


# RP6502 built-in 16-color ANSI palette.
# Index 0 is transparent, so opaque pixels are matched
# only against indices 1..15.
ANSI = [
    (0,   0,   0),       # 0  transparent
    (170, 0,   0),       # 1  red
    (0,   170, 0),       # 2  green
    (170, 85,  0),       # 3  yellow/brown
    (0,   0,   170),     # 4  blue
    (170, 0,   170),     # 5  magenta
    (0,   170, 170),     # 6  cyan
    (170, 170, 170),     # 7  light gray
    (85,  85,  85),      # 8  dark gray
    (255, 85,  85),      # 9  bright red
    (85,  255, 85),      # 10 bright green
    (255, 255, 85),      # 11 bright yellow
    (85,  85,  255),     # 12 bright blue
    (255, 85,  255),     # 13 bright magenta
    (85,  255, 255),     # 14 bright cyan
    (255, 255, 255),     # 15 white
]


def sanitize_identifier(name):
    """
    Convert arbitrary text into a valid C identifier.
    """
    name = re.sub(r"[^A-Za-z0-9_]", "_", name)

    if not name:
        name = "tileset"

    if name[0].isdigit():
        name = "_" + name

    return name


def make_macro(name):
    return sanitize_identifier(name).upper()


def nearest_ansi_index(r, g, b):
    """
    Find the closest opaque ANSI palette entry.

    Index 0 is excluded because RP6502 uses it for transparency.
    """
    best_index = 1
    best_distance = None

    for index in range(1, 16):
        pr, pg, pb = ANSI[index]

        dr = r - pr
        dg = g - pg
        db = b - pb

        distance = (
            dr * dr +
            dg * dg +
            db * db
        )

        if best_distance is None or distance < best_distance:
            best_distance = distance
            best_index = index

    return best_index


def convert_pixel(pixel):
    r, g, b, a = pixel

    if a < 128:
        return 0

    return nearest_ansi_index(r, g, b)


def pack_4bpp(first, second):
    """
    Pack two 4-bit pixels into one byte.

    High nibble = first pixel
    Low nibble  = second pixel
    """
    return (
        ((first & 0x0F) << 4) |
        (second & 0x0F)
    )


def is_blank_tile(image, x0, y0):
    """
    A tile is blank if every pixel is transparent.
    """
    for y in range(TILE_H):
        for x in range(TILE_W):
            _, _, _, a = image.getpixel(
                (x0 + x, y0 + y)
            )

            if a != 0:
                return False

    return True


def convert_tile(image, x0, y0):
    """
    Convert one 16x16 tile to packed 4-bpp.

    16 pixels/row -> 8 bytes
    16 rows       -> 128 bytes
    """
    output = bytearray()

    for y in range(TILE_H):
        for x in range(0, TILE_W, 2):
            p1 = convert_pixel(
                image.getpixel((x0 + x, y0 + y))
            )

            p2 = convert_pixel(
                image.getpixel((x0 + x + 1, y0 + y))
            )

            output.append(
                pack_4bpp(p1, p2)
            )

    return output


def convert_tileset(image):
    width, height = image.size

    if width % TILE_W != 0:
        raise ValueError(
            f"Image width {width} is not a multiple of {TILE_W}"
        )

    if height % TILE_H != 0:
        raise ValueError(
            f"Image height {height} is not a multiple of {TILE_H}"
        )

    tiles_across = width // TILE_W
    tiles_down = height // TILE_H

    output = bytearray()

    # Original source tile index -> packed runtime tile index.
    #
    # None means the source tile was blank and was omitted.
    tile_map = {}

    source_index = 0
    packed_index = 0

    for tile_y in range(tiles_down):
        for tile_x in range(tiles_across):

            x0 = tile_x * TILE_W
            y0 = tile_y * TILE_H

            if is_blank_tile(image, x0, y0):
                tile_map[source_index] = None

            else:
                tile_map[source_index] = packed_index

                tile_data = convert_tile(
                    image,
                    x0,
                    y0
                )

                output.extend(tile_data)

                packed_index += 1

            source_index += 1

    return (
        output,
        tile_map,
        tiles_across,
        tiles_down
    )


def write_header(
    path,
    array_name,
    packed_tile_count
):
    array_name = sanitize_identifier(array_name)
    prefix = make_macro(array_name)
    guard = f"{prefix}_H"

    total_bytes = packed_tile_count * 128

    with open(path, "w") as f:
        f.write(f"#ifndef {guard}\n")
        f.write(f"#define {guard}\n\n")

        f.write("#include <stdint.h>\n\n")

        f.write(
            f"#define {prefix}_TILE_WIDTH {TILE_W}\n"
        )

        f.write(
            f"#define {prefix}_TILE_HEIGHT {TILE_H}\n"
        )

        f.write(
            f"#define {prefix}_BPP 4\n"
        )

        f.write(
            f"#define {prefix}_BYTES_PER_TILE 128\n"
        )

        f.write(
            f"#define {prefix}_TILE_COUNT {packed_tile_count}\n"
        )

        f.write(
            f"#define {prefix}_TOTAL_BYTES {total_bytes}\n"
        )

        f.write("\n")

        f.write(
            f"extern const uint8_t "
            f"{array_name}[{prefix}_TOTAL_BYTES];\n"
        )

        f.write("\n#endif\n")


def write_c_file(
    path,
    header_filename,
    data,
    array_name,
    packed_tile_count
):
    array_name = sanitize_identifier(array_name)
    prefix = make_macro(array_name)

    bytes_per_tile = 128

    with open(path, "w") as f:

        f.write(
            f'#include "{header_filename}"\n\n'
        )

        f.write(
            f"const uint8_t "
            f"{array_name}[{prefix}_TOTAL_BYTES] = {{\n"
        )

        for tile_index in range(packed_tile_count):

            f.write(
                f"\n    /* tile {tile_index} */\n"
            )

            start = tile_index * bytes_per_tile
            end = start + bytes_per_tile

            tile_data = data[start:end]

            # 8 bytes per 16-pixel row at 4-bpp.
            for i in range(0, len(tile_data), 8):
                row = tile_data[i:i + 8]

                values = ", ".join(
                    f"0x{x:02X}"
                    for x in row
                )

                f.write(
                    f"    {values},\n"
                )

        f.write("};\n")


def write_mapping(
    path,
    source_file,
    tile_map,
    tiles_across,
    tiles_down
):
    packed_tile_count = sum(
        1
        for value in tile_map.values()
        if value is not None
    )

    mapping = {
        "format": "RP6502 Mode 2 tile data",
        "bpp": 4,

        "source": str(source_file),

        "tile_width": TILE_W,
        "tile_height": TILE_H,
        "bytes_per_tile": 128,

        "source_columns": tiles_across,
        "source_rows": tiles_down,
        "source_tile_count": (
            tiles_across * tiles_down
        ),

        "packed_tile_count": packed_tile_count,
        "total_bytes": packed_tile_count * 128,

        "tiles": {
            str(source): packed
            for source, packed in tile_map.items()
        }
    }

    with open(path, "w") as f:
        json.dump(
            mapping,
            f,
            indent=2
        )


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Convert a 16x16 tileset PNG into "
            "packed RP6502 4-bpp Mode 2 tile data."
        )
    )

    parser.add_argument(
        "input",
        help="Input PNG tileset"
    )

    parser.add_argument(
        "-o",
        "--output",
        required=True,
        help=(
            "Output base path. "
            "Example: build/world_tiles"
        )
    )

    parser.add_argument(
        "--name",
        default="world_tiles",
        help="Generated C array name"
    )

    args = parser.parse_args()

    input_path = Path(args.input)

    if not input_path.exists():
        raise FileNotFoundError(
            f"Input does not exist: {input_path}"
        )

    image = Image.open(
        input_path
    ).convert("RGBA")

    data, tile_map, across, down = convert_tileset(
        image
    )

    image.close()

    total_source_tiles = across * down

    packed_tile_count = sum(
        1
        for value in tile_map.values()
        if value is not None
    )

    blank_tile_count = (
        total_source_tiles - packed_tile_count
    )

    base_path = Path(args.output)

    # Allow either:
    #
    #   -o build/world_tiles
    #
    # or:
    #
    #   -o build/world_tiles.c
    #
    # Both generate .c/.h/.json.
    if base_path.suffix:
        base_path = base_path.with_suffix("")

    c_path = base_path.with_suffix(".c")
    h_path = base_path.with_suffix(".h")
    json_path = base_path.with_suffix(".json")

    c_path.parent.mkdir(
        parents=True,
        exist_ok=True
    )

    write_header(
        h_path,
        args.name,
        packed_tile_count
    )

    write_c_file(
        c_path,
        h_path.name,
        data,
        args.name,
        packed_tile_count
    )

    write_mapping(
        json_path,
        input_path,
        tile_map,
        across,
        down
    )

    print(
        f"Source tileset:   "
        f"{across} x {down}"
    )

    print(
        f"Source tiles:     "
        f"{total_source_tiles}"
    )

    print(
        f"Packed tiles:     "
        f"{packed_tile_count}"
    )

    print(
        f"Blank tiles:      "
        f"{blank_tile_count}"
    )

    print(
        f"Bytes per tile:   "
        f"128"
    )

    print(
        f"Total bytes:      "
        f"{len(data)}"
    )

    print()

    print(
        f"C source:         {c_path}"
    )

    print(
        f"C header:         {h_path}"
    )

    print(
        f"Mapping:          {json_path}"
    )


if __name__ == "__main__":
    main()