#include "collision.h"

#include "map.h"
#include "../game/game.h"
#include "../../generated/world01_tiles.h"

bool collision_check_tiles(const Entity *entity)
{
    uint16_t left_col;
    uint16_t right_col;
    uint16_t top_row;
    uint16_t bottom_row;

    int16_t left = entity->world_x + entity->hitbox.x;

    int16_t right = left + entity->hitbox.width - 1;

    int16_t top = entity->world_y + entity->hitbox.y;

    int16_t bottom = top + entity->hitbox.height - 1;

    switch(entity->direction) {

    case DIR_UP:
        top -= entity->speed;
        break;

    case DIR_DOWN:
        bottom += entity->speed;
        break;

    case DIR_LEFT:
        left -= entity->speed;
        break;

    case DIR_RIGHT:
        right += entity->speed;
        break;

    case DIR_UP_LEFT:
        /* Match the unscaled diagonal step in player_update(). */
        left -= entity->speed;
        top -= entity->speed;
        break;

    case DIR_DOWN_LEFT:
        left -= entity->speed;
        bottom += entity->speed;
        break;

    case DIR_UP_RIGHT:
        right += entity->speed;
        top -= entity->speed;
        break;

    case DIR_DOWN_RIGHT:
        right += entity->speed;
        bottom += entity->speed;
        break;

    }

    if (left < 0 || top < 0 ||
        right >= WORLD01_TILES_MAP_TOTAL_X || bottom >= WORLD01_TILES_MAP_TOTAL_Y) {
        return true;
    }

    left_col = left / TILE_SIZE;
    right_col = right / TILE_SIZE;
    top_row = top / TILE_SIZE;
    bottom_row = bottom / TILE_SIZE;

    return
        map_tile_is_solid(left_col, top_row) ||
        map_tile_is_solid(right_col, top_row) ||
        map_tile_is_solid(left_col, bottom_row) ||
        map_tile_is_solid(right_col, bottom_row);

}
