# Tileset converter

Requires Python and Pillow (`python -m pip install Pillow`).

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
