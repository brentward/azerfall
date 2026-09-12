#include "map.h"

bool tile_collision[6] = {false, true, true, false, true, false};

bool map_tile_is_solid(uint16_t col, uint16_t row)
{
    uint8_t tile;

    if (col >= WORLD01_TILES_MAP_WIDTH ||
        row >= WORLD01_TILES_MAP_HEIGHT) {
        return true;
    }

    tile = world01_tiles_map[col + row * WORLD01_TILES_MAP_WIDTH];

    if (tile >= WORLD01_TILES_TILE_COUNT) {
        return true;
    }

    return tile_collision[tile];
}