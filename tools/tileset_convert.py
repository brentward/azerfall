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


def read_world_map(path):
    rows = []
    for line_number, line in enumerate(Path(path).read_text().splitlines(), 1):
        if not line.strip():
            continue
        try:
            row = [int(value) for value in line.split()]
        except ValueError:
            raise ValueError(f"Map line {line_number}: expected space-separated integers")
        if any(value < 0 for value in row):
            raise ValueError(f"Map line {line_number}: IDs must be nonnegative")
        if rows and len(row) != len(rows[0]):
            raise ValueError(f"Map line {line_number}: inconsistent row width")
        rows.append(row)
    if not rows:
        raise ValueError("World map is empty")
    return rows


def parse_tile_mapping(value):
    mapping = {}
    for pair in value.split(","):
        try:
            map_id, source_id = [int(part.strip()) for part in pair.split("=")]
        except ValueError:
            raise ValueError("Mapping must use MAP_ID=PNG_INDEX pairs, e.g. 0=0,1=716")
        if map_id < 0 or source_id < 0:
            raise ValueError("Mapping IDs must be nonnegative")
        if map_id in mapping:
            raise ValueError(f"Duplicate mapping for map ID {map_id}")
        mapping[map_id] = source_id
    return mapping


def convert_tileset(image, selected_sources=None):
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

    if selected_sources is not None:
        # Preserve requested order, including explicitly selected blank tiles.
        tile_map = {}
        for packed_index, source_index in enumerate(selected_sources):
            if not 0 <= source_index < tiles_across * tiles_down:
                raise ValueError(f"PNG tile index {source_index} is out of range (0..{tiles_across * tiles_down - 1})")
            x0 = (source_index % tiles_across) * TILE_W
            y0 = (source_index // tiles_across) * TILE_H
            output.extend(convert_tile(image, x0, y0))
            tile_map.setdefault(source_index, packed_index)
        return output, tile_map, tiles_across, tiles_down

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
    packed_tile_count,
    map_rows=None
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

        if map_rows is not None:
            f.write(f"\n#define {prefix}_MAP_WIDTH {len(map_rows[0])}\n")
            f.write(f"#define {prefix}_MAP_HEIGHT {len(map_rows)}\n")
            f.write(f"#define {prefix}_MAP_TOTAL_BYTES {len(map_rows[0]) * len(map_rows)}\n")
            f.write(f"extern const uint8_t {array_name}_map[{prefix}_MAP_TOTAL_BYTES];\n")

        f.write("\n#endif\n")


def write_c_file(
    path,
    header_filename,
    data,
    array_name,
    packed_tile_count,
    map_rows=None
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

        if map_rows is not None:
            f.write(f"\n/* Map cells in row-major order: index = y * {prefix}_MAP_WIDTH + x. */\n")
            f.write(f"const uint8_t {array_name}_map[{prefix}_MAP_TOTAL_BYTES] = {{\n")
            for row in map_rows:
                f.write("    " + ", ".join(str(value) for value in row) + ",\n")
            f.write("};\n")


def write_mapping(
    path,
    source_file,
    tile_map,
    tiles_across,
    tiles_down,
    world_metadata=None
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

    if world_metadata is not None:
        mapping.update(world_metadata)

    with open(path, "w") as f:
        json.dump(
            mapping,
            f,
            indent=2
        )


def read_tile_data(path):
    """Read alternating numeric PNG filenames and collision booleans."""
    lines = Path(path).read_text(encoding="utf-8-sig").splitlines()
    if not lines or len(lines) % 2:
        raise ValueError("Tile data must contain filename/boolean line pairs")
    entries = {}
    for offset in range(0, len(lines), 2):
        filename, solid = [line.strip() for line in lines[offset:offset + 2]]
        if not re.fullmatch(r"[0-9]+(?:\.png)?", filename):
            raise ValueError(f"Tile data line {offset + 1}: expected a numeric PNG filename")
        index = int(Path(filename).stem)
        if index in entries:
            raise ValueError(f"Duplicate tile ID {index}")
        if solid.lower() not in ("true", "false"):
            raise ValueError(f"Tile data line {offset + 2}: expected true or false")
        entries[index] = (filename, solid.lower() == "true")
    if len(entries) > 256 or sorted(entries) != list(range(len(entries))):
        raise ValueError("Tile IDs must be contiguous from 0, with at most 256 tiles")
    return [entries[index] for index in range(len(entries))]


def read_gimp_palette(path):
    lines = Path(path).read_text(encoding="utf-8-sig").splitlines()
    if not lines or lines[0].strip() != "GIMP Palette":
        raise ValueError("Expected a GIMP Palette (.gpl) file")
    colors = []
    for line in lines[1:]:
        line = line.strip()
        if not line or line.startswith(("#", "Name:", "Columns:")):
            continue
        parts = line.split()
        if len(parts) < 3:
            raise ValueError("Invalid GIMP palette color")
        color = tuple(int(part) for part in parts[:3])
        if any(v < 0 or v > 255 for v in color):
            raise ValueError("Palette channels must be in 0..255")
        colors.append(color)
    if len(colors) != 16:
        raise ValueError(f"WARNING: expected 16 palette entries, got {len(colors)}")
    if colors[0] != (0, 0, 0):
        raise ValueError("Palette index 0 must be black (reserved for transparency)")
    return colors


def explicit_palette(image, path):
    colors = read_gimp_palette(path)
    words = [((b >> 3) << 11) | ((g >> 3) << 6) | (r >> 3)
             | (0x20 if i else 0) for i, (r, g, b) in enumerate(colors)]
    lookup = {color: i for i, color in reversed(list(enumerate(colors)))}
    pixels = []
    for r, g, b, a in image.convert("RGBA").getdata():
        if a < 128:
            pixels.append(0)
        elif (r, g, b) in lookup:
            pixels.append(lookup[(r, g, b)])
        else:
            raise ValueError(f"Color {(r, g, b)} is absent from the supplied palette")
    indexed = Image.new("L", image.size)
    indexed.putdata(pixels)
    return indexed, words


def custom_palette(image):
    """Return indexed pixels and 16 RP6502 RGB555/alpha palette words.

    Preserve PNG indices when they fit; RGBA exports use first-seen colors.
    See https://picocomputer.github.io/vga.html for the hardware format.
    """
    rgba = image.convert("RGBA")
    colors = list(dict.fromkeys(rgba.getdata()))
    if len(colors) > 16:
        raise ValueError(f"WARNING: image uses {len(colors)} colors; 4-bpp supports at most 16. Reduce the palette and export again.")
    if image.mode == "P" and max(image.getdata()) < 16:
        indexed = image.copy()
        palette = [(0, 0, 0, 0)] * 16
        for index, color in zip(image.getdata(), rgba.getdata()):
            palette[index] = color
    else:
        palette = colors + [(0, 0, 0, 0)] * (16 - len(colors))
        lookup = {color: index for index, color in enumerate(colors)}
        indexed = Image.new("L", image.size)
        indexed.putdata([lookup[color] for color in rgba.getdata()])
    if any(a not in (0, 255) for _, _, _, a in colors):
        import warnings
        warnings.warn("Partial alpha uses a binary threshold of 128 on RP6502")
    words = [((b >> 3) << 11) | ((g >> 3) << 6) | (r >> 3)
             | (0x20 if a >= 128 else 0) for r, g, b, a in palette]
    return indexed, words


def convert_directory(args):
    if not args.tile_data or args.mapping:
        raise ValueError("Directory input requires --tile-data and does not use --mapping")
    entries = read_tile_data(args.tile_data)
    data = bytearray()
    palette = None
    sheet = not Path(args.input).is_dir()
    if sheet:
        with Image.open(args.input) as png:
            width, height = png.size
            if width % TILE_W or height % TILE_H:
                raise ValueError("Sheet dimensions must be multiples of 16")
            if len(entries) > (width // TILE_W) * (height // TILE_H):
                raise ValueError("Tile data refers to indexes outside the sheet")
            indexed, palette = (explicit_palette(png, args.palette) if args.palette
                                else custom_palette(png))
            for index in range(len(entries)):
                x0 = (index % (width // TILE_W)) * TILE_W
                y0 = (index // (width // TILE_W)) * TILE_H
                for y in range(TILE_H):
                    for x in range(0, TILE_W, 2):
                        data.append(pack_4bpp(indexed.getpixel((x0 + x, y0 + y)),
                                             indexed.getpixel((x0 + x + 1, y0 + y))))
    else:
        for filename, _ in entries:
            with Image.open(Path(args.input) / filename) as png:
                if png.size != (TILE_W, TILE_H):
                    raise ValueError(f"{filename}: expected {TILE_W}x{TILE_H}, got {png.size}")
                data.extend(convert_tile(png.convert("RGBA"), 0, 0))

    base = Path(args.output).with_suffix("")
    name = sanitize_identifier(args.name)
    prefix = make_macro(name)
    maps = []
    stems = set()
    for path in args.world_map or []:
        stem = sanitize_identifier(Path(path).stem)
        if stem.upper() in stems:
            raise ValueError(f"Map output name collision: {path}")
        stems.add(stem.upper())
        rows = read_world_map(path)
        missing = {value for row in rows for value in row if value >= len(entries)}
        if missing:
            raise ValueError(f"{path}: undefined tile IDs {sorted(missing)}")
        maps.append((path, stem, rows))

    # Validate every input before writing outputs. All maps retain source IDs.
    base.parent.mkdir(parents=True, exist_ok=True)
    header = base.with_suffix(".h")
    source = base.with_suffix(".c")
    write_header(header, name, len(entries))
    declaration = f"extern const uint8_t {name}_collision[{prefix}_TILE_COUNT];\n\n"
    if palette is not None:
        declaration += (f"#define {prefix}_PALETTE_COUNT 16\n"
                        f"#define {prefix}_PALETTE_BYTES ({prefix}_PALETTE_COUNT * 2)\n"
                        f"extern const uint16_t {name}_palette[{prefix}_PALETTE_COUNT];\n\n")
    header.write_text(header.read_text().replace("#endif", declaration + "#endif"))
    write_c_file(source, header.name, data, name, len(entries))
    with source.open("a") as output:
        if palette is not None:
            output.write(f"\nconst uint16_t {name}_palette[{prefix}_PALETTE_COUNT] = {{\n    ")
            output.write(", ".join(f"0x{word:04X}" for word in palette))
            output.write("\n};\n")
        output.write(f"\nconst uint8_t {name}_collision[{prefix}_TILE_COUNT] = {{\n    ")
        output.write(", ".join(str(int(solid)) for _, solid in entries))
        output.write("\n};\n")
    metadata = {
        "source": str(args.input), "tile_data": str(args.tile_data),
        "bpp": 4, "tile_width": TILE_W, "tile_height": TILE_H,
        "bytes_per_tile": 128, "packed_tile_count": len(entries),
        "total_bytes": len(data),
        "tiles": {str(i): {"filename": filename, "runtime_id": i, "collision": solid}
                  for i, (filename, solid) in enumerate(entries)},
        "maps": [],
    }
    if sheet:
        metadata["palette"] = palette
        metadata["palette_format"] = "RP6502 RGB555, alpha bit 5"
        if args.palette:
            metadata["palette_source"] = str(args.palette)
            metadata["transparent_rgb"] = [0, 0, 0]
        metadata["source_columns"] = width // TILE_W
        metadata["source_rows"] = height // TILE_H
        for index, tile in enumerate(metadata["tiles"].values()):
            tile.pop("filename")
            tile["source_index"] = index
    for path, stem, rows in maps:
        map_base = base.parent / f"{base.name}_{stem}_map"
        map_name = f"{name}_{stem}_map"
        macro = make_macro(map_name)
        width, height = len(rows[0]), len(rows)
        map_base.with_suffix(".h").write_text(
            f"#ifndef {macro}_H\n#define {macro}_H\n\n"
            f'#include "{header.name}"\n\n'
            f"#define {macro}_WIDTH {width}\n#define {macro}_HEIGHT {height}\n"
            f"#define {macro}_TOTAL_BYTES {width * height}\n"
            f"#define {macro}_TOTAL_X ({macro}_WIDTH * {prefix}_TILE_WIDTH)\n"
            f"#define {macro}_TOTAL_Y ({macro}_HEIGHT * {prefix}_TILE_HEIGHT)\n"
            f"extern const uint8_t {map_name}[{macro}_TOTAL_BYTES];\n\n#endif\n")
        map_base.with_suffix(".c").write_text(
            f'#include "{map_base.name}.h"\n\n'
            f"const uint8_t {map_name}[{macro}_TOTAL_BYTES] = {{\n" +
            "".join("    " + ", ".join(map(str, row)) + ",\n" for row in rows) + "};\n")
        map_base.with_suffix(".txt").write_text(
            "".join(" ".join(map(str, row)) + "\n" for row in rows))
        metadata["maps"].append({"source": str(path), "array": map_name,
                                 "width": width, "height": height})
    base.with_suffix(".json").write_text(json.dumps(metadata, indent=2) + "\n")
    print(f"Generated {len(entries)} shared tiles ({len(data)} bytes), collision flags, and {len(maps)} maps in {base.parent}")


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Convert a 16x16 tileset PNG into "
            "packed RP6502 4-bpp Mode 2 tile data."
        )
    )

    parser.add_argument(
        "input",
        help="Input PNG tileset or directory of numbered 16x16 PNG tiles"
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

    parser.add_argument("--map", dest="world_map", action="append", help="Space-separated map TXT file; repeat for multiple maps with directory input")
    parser.add_argument("--tile-data", help="Alternating tile index/collision boolean TXT file; numeric PNG names also accepted")
    parser.add_argument("--mapping", help="Map ID to original PNG tile index, e.g. 0=0,1=716,2=320 (zero-based, left to right then top to bottom)")
    parser.add_argument("--palette", help="GIMP palette: 16 ordered colors, index 0 transparent black; requires sheet and --tile-data")
    args = parser.parse_args()
    if args.palette and (not args.tile_data or Path(args.input).is_dir()):
        parser.error("--palette requires PNG sheet input with --tile-data")
    if Path(args.input).is_dir() or args.tile_data:
        try:
            convert_directory(args)
        except (ValueError, OSError) as error:
            parser.error(str(error))
        return
    if args.tile_data:
        parser.error("--tile-data requires directory input")
    if args.world_map and len(args.world_map) > 1:
        parser.error("Multiple maps require directory input")
    args.world_map = args.world_map[0] if args.world_map else None
    if bool(args.world_map) != bool(args.mapping):
        parser.error("--map and --mapping must be provided together")

    rows = None
    map_rows = None
    selected_sources = None
    world_metadata = None
    try:
        if args.world_map:
            rows = read_world_map(args.world_map)
            selection = parse_tile_mapping(args.mapping)
            used_ids = sorted({value for row in rows for value in row})
            if len(used_ids) > 256:
                raise ValueError("Byte-indexed maps support at most 256 distinct map IDs")
            missing = set(used_ids) - selection.keys()
            if missing:
                raise ValueError(f"Missing mappings for map IDs: {sorted(missing)}")
            runtime_ids = {map_id: index for index, map_id in enumerate(used_ids)}
            map_rows = [[runtime_ids[value] for value in row] for row in rows]
            selected_sources = [selection[map_id] for map_id in used_ids]
            world_metadata = {
                "world_map": str(args.world_map),
                "map_width": len(rows[0]),
                "map_height": len(rows),
                "map_id_to_source": {str(i): selection[i] for i in used_ids},
                "map_id_to_runtime": {str(i): runtime_ids[i] for i in used_ids},
                "packed_tile_count": len(used_ids),
                "total_bytes": len(used_ids) * 128,
            }
    except (ValueError, OSError) as error:
        parser.error(str(error))

    input_path = Path(args.input)

    if not input_path.exists():
        raise FileNotFoundError(
            f"Input does not exist: {input_path}"
        )

    image = Image.open(
        input_path
    ).convert("RGBA")

    try:
        data, tile_map, across, down = convert_tileset(image, selected_sources)
    except ValueError as error:
        parser.error(str(error))

    image.close()

    total_source_tiles = across * down

    packed_tile_count = len(data) // 128

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
        packed_tile_count,
        map_rows
    )

    write_c_file(
        c_path,
        h_path.name,
        data,
        args.name,
        packed_tile_count,
        map_rows
    )

    write_mapping(
        json_path,
        input_path,
        tile_map,
        across,
        down,
        world_metadata
    )

    if rows is not None:
        map_path = base_path.with_suffix(".map.txt")
        map_path.write_text("".join(
            " ".join(str(value) for value in row) + "\n"
            for row in map_rows
        ))
        print(f"Runtime map:      {map_path}")

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
        f"Omitted tiles:    "
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
