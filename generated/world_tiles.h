#ifndef WORLD_TILES_H
#define WORLD_TILES_H

#include <stdint.h>

#define WORLD_TILES_TILE_WIDTH 16
#define WORLD_TILES_TILE_HEIGHT 16
#define WORLD_TILES_BPP 4
#define WORLD_TILES_BYTES_PER_TILE 128
#define WORLD_TILES_TILE_COUNT 38
#define WORLD_TILES_TOTAL_BYTES 4864

extern const uint8_t world_tiles[WORLD_TILES_TOTAL_BYTES];

extern const uint8_t world_tiles_collision[WORLD_TILES_TILE_COUNT];

#define WORLD_TILES_PALETTE_COUNT 16
#define WORLD_TILES_PALETTE_BYTES (WORLD_TILES_PALETTE_COUNT * 2)
extern const uint16_t world_tiles_palette[WORLD_TILES_PALETTE_COUNT];

#endif
