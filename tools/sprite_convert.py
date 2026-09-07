from PIL import Image
from pathlib import Path
import argparse
import json
import re


SPRITE_W = 16
SPRITE_H = 16


# RP6502 built-in 16-color ANSI palette.
# Index 0 is transparent, so opaque pixels are matched
# against indices 1..15 only.
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
    name = re.sub(r"[^A-Za-z0-9_]", "_", name)

    if not name:
        name = "sprites"

    if name[0].isdigit():
        name = "_" + name

    return name


def make_macro(name):
    return sanitize_identifier(name).upper()


def parse_rgb(value):
    """
    Parse R,G,B.
    Example:
        --transparent 48,104,80
    """
    try:
        parts = [int(x.strip()) for x in value.split(",")]
    except ValueError:
        raise argparse.ArgumentTypeError(
            "Color must be R,G,B"
        )

    if len(parts) != 3:
        raise argparse.ArgumentTypeError(
            "Color must contain exactly three values: R,G,B"
        )

    if any(x < 0 or x > 255 for x in parts):
        raise argparse.ArgumentTypeError(
            "Color values must be from 0 through 255"
        )

    return tuple(parts)


def parse_region(value):
    """
    Parse x,y,width,height.
    Example:
        --region 0,0,64,128
    """
    try:
        parts = [int(x.strip()) for x in value.split(",")]
    except ValueError:
        raise argparse.ArgumentTypeError(
            "Region must be x,y,width,height"
        )

    if len(parts) != 4:
        raise argparse.ArgumentTypeError(
            "Region must contain four values: x,y,width,height"
        )

    x, y, w, h = parts

    if x < 0 or y < 0 or w <= 0 or h <= 0:
        raise argparse.ArgumentTypeError(
            "Region must have nonnegative x/y and positive width/height"
        )

    return tuple(parts)


def nearest_ansi_index(r, g, b):
    """
    Find closest opaque ANSI entry.

    Index 0 is excluded because RP6502's built-in
    palette uses it for transparency.
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


def is_transparent(pixel, transparent_rgb):
    """
    Transparent if:
      - PNG alpha < 128, or
      - RGB exactly equals selected background color.
    """
    r, g, b, a = pixel

    if a < 128:
        return True

    if transparent_rgb is not None:
        if (r, g, b) == transparent_rgb:
            return True

    return False


def convert_pixel(pixel, transparent_rgb):
    if is_transparent(pixel, transparent_rgb):
        return 0

    r, g, b, _ = pixel

    return nearest_ansi_index(r, g, b)


def pack_4bpp(first, second):
    """
    Two pixels per byte.

        high nibble = first pixel
        low nibble  = second pixel
    """
    return (
        ((first & 0x0F) << 4) |
        (second & 0x0F)
    )


def is_blank_sprite(
    image,
    x0,
    y0,
    transparent_rgb
):
    for y in range(SPRITE_H):
        for x in range(SPRITE_W):
            pixel = image.getpixel(
                (x0 + x, y0 + y)
            )

            if not is_transparent(
                pixel,
                transparent_rgb
            ):
                return False

    return True


def convert_sprite(
    image,
    x0,
    y0,
    transparent_rgb
):
    """
    Convert one 16x16 cell to packed 4-bpp.

    16x16 = 256 pixels
    2 pixels/byte = 128 bytes
    """
    output = bytearray()

    for y in range(SPRITE_H):
        for x in range(0, SPRITE_W, 2):

            p1 = convert_pixel(
                image.getpixel(
                    (x0 + x, y0 + y)
                ),
                transparent_rgb
            )

            p2 = convert_pixel(
                image.getpixel(
                    (x0 + x + 1, y0 + y)
                ),
                transparent_rgb
            )

            output.append(
                pack_4bpp(p1, p2)
            )

    return output


def extract_sprites(
    image,
    region,
    transparent_rgb,
    skip_blank
):
    if region is None:
        region_x = 0
        region_y = 0
        region_w = image.width
        region_h = image.height
    else:
        region_x, region_y, region_w, region_h = region

    if region_x + region_w > image.width:
        raise ValueError(
            "Region extends beyond image width"
        )

    if region_y + region_h > image.height:
        raise ValueError(
            "Region extends beyond image height"
        )

    if region_w % SPRITE_W != 0:
        raise ValueError(
            f"Region width {region_w} is not divisible "
            f"by {SPRITE_W}"
        )

    if region_h % SPRITE_H != 0:
        raise ValueError(
            f"Region height {region_h} is not divisible "
            f"by {SPRITE_H}"
        )

    columns = region_w // SPRITE_W
    rows = region_h // SPRITE_H

    data = bytearray()
    sprites = []

    source_cell = 0

    for row in range(rows):
        for col in range(columns):

            x0 = region_x + col * SPRITE_W
            y0 = region_y + row * SPRITE_H

            blank = is_blank_sprite(
                image,
                x0,
                y0,
                transparent_rgb
            )

            if skip_blank and blank:
                source_cell += 1
                continue

            sprite_data = convert_sprite(
                image,
                x0,
                y0,
                transparent_rgb
            )

            index = len(sprites)
            offset = len(data)

            sprites.append({
                "index": index,
                "source_cell": source_cell,
                "column": col,
                "row": row,
                "x": x0,
                "y": y0,
                "offset": offset,
                "bytes": len(sprite_data),
                "blank": blank
            })

            data.extend(sprite_data)

            source_cell += 1

    return data, sprites, columns, rows


def make_sprite_name(sprite, prefix):
    """
    Generate names based on row and frame.

    Example:
        player_walk_dir0_frame0
        player_walk_dir0_frame1
        player_walk_dir1_frame0
    """
    return (
        f"{prefix}_dir{sprite['row']}"
        f"_frame{sprite['column']}"
    )


def write_header(
    path,
    array_name,
    sprite_prefix,
    sprites
):
    array_name = sanitize_identifier(array_name)
    prefix = make_macro(array_name)

    guard = f"{prefix}_H"

    with open(path, "w") as f:
        f.write(f"#ifndef {guard}\n")
        f.write(f"#define {guard}\n\n")

        f.write("#include <stdint.h>\n\n")

        f.write(
            f"#define {prefix}_WIDTH {SPRITE_W}\n"
        )

        f.write(
            f"#define {prefix}_HEIGHT {SPRITE_H}\n"
        )

        f.write(
            f"#define {prefix}_BPP 4\n"
        )

        f.write(
            f"#define {prefix}_BYTES_PER_SPRITE 128\n"
        )

        f.write(
            f"#define {prefix}_COUNT {len(sprites)}\n"
        )

        f.write(
            f"#define {prefix}_TOTAL_BYTES "
            f"{len(sprites) * 128}\n"
        )

        f.write("\n")

        for sprite in sprites:
            name = make_sprite_name(
                sprite,
                sprite_prefix
            )

            macro = make_macro(name)

            f.write(
                f"#define {macro} "
                f"{sprite['index']}\n"
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
    array_name,
    sprite_prefix,
    data,
    sprites
):
    array_name = sanitize_identifier(array_name)
    prefix = make_macro(array_name)

    sprite_offsets = {
        sprite["offset"]: make_sprite_name(
            sprite,
            sprite_prefix
        )
        for sprite in sprites
    }

    with open(path, "w") as f:

        f.write(
            f'#include "{header_filename}"\n\n'
        )

        f.write(
            f"const uint8_t "
            f"{array_name}[{prefix}_TOTAL_BYTES] = {{\n"
        )

        for i in range(0, len(data), 16):

            if i in sprite_offsets:
                f.write(
                    f"\n    /* "
                    f"{sprite_offsets[i]} "
                    f"*/\n"
                )

            chunk = data[i:i + 16]

            values = ", ".join(
                f"0x{x:02X}"
                for x in chunk
            )

            f.write(
                f"    {values},\n"
            )

        f.write("};\n")


def write_json(
    path,
    input_path,
    region,
    transparent_rgb,
    sprites,
    columns,
    rows,
    sprite_prefix
):
    entries = []

    for sprite in sprites:
        entry = dict(sprite)

        entry["name"] = make_sprite_name(
            sprite,
            sprite_prefix
        )

        entries.append(entry)

    manifest = {
        "format": "RP6502 Mode 5 sprite data",
        "bpp": 4,
        "sprite_width": SPRITE_W,
        "sprite_height": SPRITE_H,
        "bytes_per_sprite": 128,

        "source": str(input_path),

        "region": (
            list(region)
            if region is not None
            else None
        ),

        "transparent_rgb": (
            list(transparent_rgb)
            if transparent_rgb is not None
            else None
        ),

        "columns": columns,
        "rows": rows,

        "sprite_count": len(sprites),
        "total_bytes": len(sprites) * 128,

        "sprites": entries
    }

    with open(path, "w") as f:
        json.dump(
            manifest,
            f,
            indent=2
        )


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Convert a 16x16 sprite sheet into "
            "RP6502 packed 4-bpp Mode 5 data."
        )
    )

    parser.add_argument(
        "input",
        help="Input PNG sprite sheet"
    )

    parser.add_argument(
        "-o",
        "--output",
        required=True,
        help=(
            "Output base path. "
            "Example: build/player_sprites"
        )
    )

    parser.add_argument(
        "--name",
        default="player_sprites",
        help="Generated C array name"
    )

    parser.add_argument(
        "--prefix",
        default="player_walk",
        help=(
            "Prefix for generated sprite constants. "
            "Default: player_walk"
        )
    )

    parser.add_argument(
        "--region",
        type=parse_region,
        help=(
            "Only convert a rectangular region: "
            "x,y,width,height"
        )
    )

    parser.add_argument(
        "--transparent",
        type=parse_rgb,
        help=(
            "RGB color to treat as transparent. "
            "Example: 48,104,80"
        )
    )

    parser.add_argument(
        "--skip-blank",
        action="store_true",
        help="Do not output completely blank cells"
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

    print(
        f"Source image:      "
        f"{image.width}x{image.height}"
    )

    if args.region:
        print(
            f"Region:            "
            f"{args.region}"
        )
    else:
        print(
            "Region:            entire image"
        )

    if args.transparent:
        print(
            f"Transparent color: "
            f"{args.transparent}"
        )
    else:
        print(
            "Transparent color: PNG alpha only"
        )

    data, sprites, columns, rows = extract_sprites(
        image,
        args.region,
        args.transparent,
        args.skip_blank
    )

    image.close()

    base_path = Path(args.output)

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
        args.prefix,
        sprites
    )

    write_c_file(
        c_path,
        h_path.name,
        args.name,
        args.prefix,
        data,
        sprites
    )

    write_json(
        json_path,
        input_path,
        args.region,
        args.transparent,
        sprites,
        columns,
        rows,
        args.prefix
    )

    print()
    print(
        f"Grid:              "
        f"{columns} columns x {rows} rows"
    )

    print(
        f"Sprites written:   "
        f"{len(sprites)}"
    )

    print(
        f"Bytes per sprite:  "
        f"128"
    )

    print(
        f"Total bytes:       "
        f"{len(data)}"
    )

    print()
    print(
        f"C source:          {c_path}"
    )

    print(
        f"C header:          {h_path}"
    )

    print(
        f"Manifest:          {json_path}"
    )


if __name__ == "__main__":
    main()