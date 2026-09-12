#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <stdbool.h>
#include "../../generated/world01_tiles.h"
#include "../game/game.h"

extern bool tile_collision[6];

bool map_tile_is_solid(uint16_t col, uint16_t row);

#endif // MAP_H