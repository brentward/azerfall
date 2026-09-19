# Sprite converter

## Objects and animated entities

Use `--layout objects` for objects. Headers use source-cell names such as
`OBJECT_IMAGE0`, with values equal to packed runtime indexes. Blank cells are
skipped by default; `--keep-blank` retains them. Source-cell numbers stay stable
when blanks occur between images, while runtime indexes are compacted.

```powershell
python tools/sprite_convert.py assets/sprites/objects/objects.png --layout objects --shared-palette generated/world_tiles.json --name object_sprites --prefix object -o generated/object_sprites
```

Use `--layout animation` (the default for compatibility) for players, NPCs, and
enemies. Each row is a direction and each column is an animation frame.
`--frames 3` exports only the first three columns of every row; without it all
columns are frames. Headers and comments retain `DIR0_FRAME0` style names and
headers expose `FRAMES_PER_DIRECTION` and `DIRECTIONS`. Blank frames are retained
by default to preserve animation slots. Explicit `--skip-blank` compacts them;
in that case use generated frame constants rather than direction/frame arithmetic.

Blank means transparent according to PNG alpha or the active transparency color
key (black for the shared overworld palette), not merely a solid-color image.

Requires Python and Pillow. Sheets use a tightly packed 16x16 grid in row-major
order. Default output remains 4-bit ANSI for compatibility with player exports.

`--palette assets/palettes/overworld.gpl` can also generate a standalone palette
with the exact GPL ordering, including unused entries. Index 0 is transparent
black. For this project's chests, use `--shared-palette` instead to avoid a second
palette array and upload. It inherits the world's transparent-black color key.

Generate a custom palette:

```powershell
python tools/sprite_convert.py assets/sprites/objects/chest.png --custom-palette --name chest_sprites --prefix chest -o generated/chest_sprites
```

This emits sprite C/header/JSON files and a 16-word `chest_sprites_palette`, with
`CHEST_SPRITES_PALETTE_COUNT` and `CHEST_SPRITES_PALETTE_BYTES` (32). Upload palette
words little-endian to aligned XRAM and set the sprite's `palette_ptr` to it.
Frame constants are sprite indexes; multiply by 128 for byte offsets.

Reuse the already generated overworld palette:

```powershell
python tools/sprite_convert.py assets/sprites/objects/chest.png --shared-palette generated/world_tiles.json --name chest_sprites --prefix chest -o generated/chest_sprites
```

Shared mode matches actual colors in RP6502 RGB555 format, not the PNG's index
numbers. It emits no palette C array: set the chest sprite's `palette_ptr` to
`BACKGROUND_PALETTE`, reusing the world's existing XRAM upload. The JSON retains
the palette values for inspection and the source manifest path; this costs no
runtime memory. Regenerate dependent sprites whenever the world palette changes.

Both custom modes reject more than 16 RGBA colors in the selected region. Shared
mode also rejects colors absent from the supplied palette, including transparency
if the palette has no transparent entry. Partial alpha uses the same threshold
of 128 as tiles, with a warning. `--transparent R,G,B` can explicitly make a
background color transparent. `--region` and `--skip-blank` remain supported.

The chest now shares the explicit overworld palette. Its opaque black pixels map to transparent index 0; white maps to index 15. No chest palette array is emitted.

Run checks with `python tools/test_sprite_convert.py`.

