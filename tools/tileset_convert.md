# Tileset converter

Requires Python and Pillow (`python -m pip install Pillow`).

The project uses `assets/palettes/overworld.gpl` as its authoritative palette.
`--palette` preserves all 16 entries and their order, including unused colors.
Black pixels and transparent pixels map to index 0 (transparent); white remains
at index 15. Colors absent from the GPL are rejected. Regenerate shared sprites
after regenerating tiles when the palette changes. The automatic-palette rules
below apply only when `--palette` is omitted.

Convert the Pixelorama sheet, collision flags, and current world map:

```powershell
python tools/tileset_convert.py "assets/tiles/Azerfall - Overworld.png" --tile-data assets/maps/tiledata.txt --palette assets/palettes/overworld.gpl --map assets/maps/worldmap.txt -o generated/world_tiles --name world_tiles
```

For sheet input, `tiledata.txt` contains alternating zero-based tile indexes and
collision booleans, each on its own line. Existing numeric names such as `000.png`
also mean tile index 0; no metadata rewrite is necessary. Indexes run left to
right, then top to bottom in the 16x16 grid. IDs must be contiguous from zero,
with at most 256 tiles. Listed blank and unused tiles retain their IDs. Extra
sheet cells are ignored for tile output.

Sheet input with `--tile-data` generates `world_tiles_palette`, a 16-entry
`uint16_t` array, `WORLD_TILES_PALETTE_COUNT` (16 entries), and
`WORLD_TILES_PALETTE_BYTES` (32 bytes). Palette words use the
[RP6502 RGB555 format with alpha at bit 5](https://picocomputer.github.io/vga.html).
The game uploads these words little-endian to its background palette in XRAM.
Opaque black is supported, including at palette index zero.

Indexed PNG pixel indexes are preserved when they fit in 0..15. RGBA exports
(including the current Pixelorama export) get a palette in first-seen pixel order;
indexed exports using higher indexes are compacted. Unused slots are transparent.
The entire sheet must use at most 16 distinct RGBA colors, including transparency.
More colors produce a warning/error and stop conversion before outputs are written.
RGB channels are reduced to the hardware's five bits; partial alpha emits a warning
and uses a threshold of 128. No automatic color quantization is performed.

The legacy directory workflow still accepts `assets/tiles` as input and uses ANSI
colors. In that workflow, `tiledata.txt` contains filename and collision lines:
`000.png`, `false`, `001.png`, `true`, each on its own line. Filenames must be
numeric PNG names with contiguous IDs starting at zero (at most 256 tiles).
Each image must be 16×16. The numeric filename determines the ID, regardless of
metadata line order. All listed tiles are exported, including transparent and
unused tiles, so map IDs remain unchanged across maps and conversion runs.
Unlisted images in the directory are ignored.

Repeat `--map` to generate multiple maps sharing the same tiles and collision
table, for example `--map assets/maps/worldmap.txt --map assets/maps/dungeon01.txt`.
Omit `--map` to generate only the tileset. Every map ID must exist in tile data.

The command above generates:

- `generated/world_tiles.c` / `.h`: shared tile pixels and
  `world_tiles_collision`, indexed by tile ID (0 = walkable, 1 = solid), plus
  the custom `world_tiles_palette` for sheet input.
- `generated/world_tiles_worldmap_map.c` / `.h`: `world_tiles_worldmap_map`
  and `WORLD_TILES_WORLDMAP_MAP_WIDTH`, `_HEIGHT`, `_TOTAL_BYTES` constants,
  plus `_TOTAL_X` and `_TOTAL_Y` for the map dimensions in pixels.
- `generated/world_tiles_worldmap_map.txt`: unchanged IDs in row-major order.
- `generated/world_tiles.json`: filenames, collision flags, IDs, and map dimensions.

Map outputs use `<output>_<map-file-stem>_map`; map stems must be unique after
C identifier sanitization (case-insensitive). Tiles are emitted once, independently
of the maps. Game integration requires compiling the generated C files and using
these arrays; conversion does not change the game's current map selection.

The PNG-grid workflow below remains supported.

Convert a whole tileset, omitting transparent tiles as before:

```powershell
python tools/tileset_convert.py assets/tiles/grass_tileset_16x16.png -o build/grass_tiles --name grass_tiles
```

Select tiles for a world map:

```powershell
python tools/tileset_convert.py assets/tiles/punyworld-overworld-tileset.png --map assets/maps/world01.txt --mapping "0=0,1=716,2=320,3=35,4=197,5=47" -o build/world01_tiles --name world01_tiles
```

The map must contain rectangular rows of whitespace-separated nonnegative integers.
PNG tile indexes are zero-based, left to right, then top to bottom, including blank
tiles. Each `MAP_ID=PNG_INDEX` pair selects a source tile. Every ID used in the map
needs a mapping; extra mappings are ignored.

Only IDs present in the map are exported, in ascending map-ID order. Explicitly
selected transparent tiles are retained. Two map IDs selecting the same PNG tile
each get their own runtime slot.

Outputs:

- `.c` and `.h`: packed 4-bpp tiles, 128 bytes per tile, and a byte-per-cell map
  array named `<name>_map` when `--map` is supplied. The header includes
  `<NAME>_MAP_WIDTH`, `<NAME>_MAP_HEIGHT`, and `<NAME>_MAP_TOTAL_BYTES`.
  Map cells are stored row by row; access a cell with `map[y * MAP_WIDTH + x]`.
  At most 256 distinct map IDs are supported.
- `.json`: source and runtime index mappings, plus map dimensions.
- `.map.txt`: map rewritten to match the compact runtime tile indexes.

For IDs 0 through 5, the runtime map has the same values as the input. For sparse
IDs such as 0 and 5, runtime indexes become 0 and 1; use the generated `.map.txt`
when loading the map. The input map is never overwritten by the default output
naming. This tool does not modify the game's map-loading code.

Run checks with `python tools/test_tileset_convert.py`.

