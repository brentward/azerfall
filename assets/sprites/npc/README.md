# NPC sprite import

The 64 by 64 sheet stores two frames per pose in row order:

| Source cells | NPC pose |
| --- | --- |
| 0, 1 | Old man down |
| 2, 3 | Old man up |
| 4, 5 | Old man left |
| 6, 7 | Old man right |
| 8, 9 | Merchant down |

Run from the repository root:

```powershell
python tools/sprite_convert.py 'assets/sprites/npc/Azerfall - NPC.png' -o generated/npc_sprites --name npc_sprites --prefix npc --layout objects --custom-palette --transparent 255,0,132
```

Object layout exports the occupied cells in row order and skips the six empty
cells. `src/entity/npc.h` maps the generated cell constants to NPC poses.
Animation layout assumes one direction per row, which this sheet does not use.

The pink background is an opaque color in the source PNG. The explicit color
key makes it transparent in the generated palette; visible black stays opaque.
With a custom palette, transparency comes from the palette word's alpha bit,
not the index number alone.
