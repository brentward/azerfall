#ifndef XRAM_LAYOUT_H
#define XRAM_LAYOUT_H

#include <rp6502.h>

#include "../generated/world_tiles_worldmap_map.h"
#include "../generated/player_sprites.h"
#include "../generated/object_sprites.h"

#define PSG_CHANNELS 8

#define PSG_WAVE_SINE 0x00
#define PSG_WAVE_SQUARE 0x10
#define PSG_WAVE_SAWTOOTH 0x20
#define PSG_WAVE_TRIANGLE 0x30
#define PSG_WAVE_NOISE 0x40

#define PSG_GATE 0x01

#define PSG_FREQ_HZ(hz) ((hz) * 3u)
#define PSG_PAN(pan) ((uint8_t)((pan) * 2))

#define xreg_ria_psg(...) xreg(0, 1, 0, __VA_ARGS__)
#define xreg_ria_opl(...) xreg(0, 1, 1, __VA_ARGS__)

#define XRAM_WORLD_MAP      0x0000U
#define XRAM_WORLD_TILES    0x1000U

#define XRAM_PLAYER_IMAGES \
    (XRAM_WORLD_TILES + WORLD_TILES_TOTAL_BYTES)

#define XRAM_OBJECT_IMAGES \
    (XRAM_PLAYER_IMAGES + PLAYER_SPRITES_TOTAL_BYTES)


/* Keep the complete song after graphics and before sound configurations. */
#define XRAM_SONG_DATA \
    ((XRAM_OBJECT_IMAGES + OBJECT_SPRITES_TOTAL_BYTES + 255UL) & 0xFF00UL)

#define XRAM_SOUND_CONFIGS      0xFBC0U
#define XRAM_SOUND_CONFIGS_SIZE 64U

#define XRAM_SONG_DATA_MAX_SIZE \
    (XRAM_SOUND_CONFIGS - XRAM_SONG_DATA)

#define XRAM_SPRITE_CONFIGS 0xFC00U
#define XRAM_SPRITE_LIMIT   32U

#define XRAM_SPRITE_CONFIG(slot) \
    (XRAM_SPRITE_CONFIGS + (slot) * sizeof(vga_mode5_sprite_t))



#define XRAM_OPL_REGISTERS  0xFD00U
#define XRAM_OPL_REGISTERS_SIZE 256U
#define XRAM_WORLD_PALETTE  0xFE00U
#define XRAM_BG_CONFIG      0xFF00U
#define XRAM_KEYBOARD       0xFF10U // 32 bytes of keypress bitmask data
#define XRAM_GAMEPAD        0xFF80U // 40 bytes of gamepad data

#if XRAM_OBJECT_IMAGES + OBJECT_SPRITES_TOTAL_BYTES > XRAM_SONG_DATA
#error Graphics data overlaps song data
#endif

#if XRAM_SONG_DATA >= XRAM_SOUND_CONFIGS
#error No XRAM space remains for song data
#endif 

#endif // XRAM_LAYOUT_H
