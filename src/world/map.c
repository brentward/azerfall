#include "map.h"

#include "../../generated/world_tiles_worldmap_map.h"


uint8_t map_tile_is_solid(uint16_t col, uint16_t row)
{
    uint8_t tile;

    if (col >= WORLD_TILES_WORLDMAP_MAP_WIDTH ||
        row >= WORLD_TILES_WORLDMAP_MAP_HEIGHT) {
        return true;
    }

    tile = world_tiles_worldmap_map[col + row * WORLD_TILES_WORLDMAP_MAP_WIDTH];

    if (tile >= WORLD_TILES_TILE_COUNT) {
        return true;
    }

    return world_tiles_collision[tile];
}