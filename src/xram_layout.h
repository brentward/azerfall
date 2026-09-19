#ifndef XRAM_LAYOUT_H
#define XRAM_LAYOUT_H

#include <rp6502.h>

#include "../generated/world_tiles_worldmap_map.h"
#include "../generated/player_sprites.h"
#include "../generated/object_sprites.h"



#define XRAM_WORLD_MAP      0x0000U
#define XRAM_WORLD_TILES    0x1000U

#define XRAM_PLAYER_IMAGES \
    (XRAM_WORLD_TILES + WORLD_TILES_TOTAL_BYTES)

#define XRAM_OBJECT_IMAGES \
    (XRAM_PLAYER_IMAGES + PLAYER_SPRITES_TOTAL_BYTES)

#define XRAM_SPRITE_CONFIGS 0xFC00U
#define XRAM_SPRITE_LIMIT   32U

#define XRAM_SPRITE_CONFIG(slot) \
    (XRAM_SPRITE_CONFIGS + (slot) * sizeof(vga_mode5_sprite_t))

#define XRAM_WORLD_PALETTE  0xFE00U
#define XRAM_BG_CONFIG     0xFF00U
#define XRAM_KEYBOARD      0xFF10U // 32 bytes of keypress bitmask data
#define XRAM_GAMEPAD       0xFF80U // 40 bytes of gamepad data

#if XRAM_OBJECT_IMAGES + OBJECT_SPRITES_TOTAL_BYTES > XRAM_SPRITE_CONFIGS
#error Graphics data overlaps sprite configurations
#endif 

#endif // XRAM_LAYOUT_H