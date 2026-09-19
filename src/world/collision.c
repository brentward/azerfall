#include "collision.h"

#include "map.h"
#include "../game/game.h"
#include "../../generated/world_tiles_worldmap_map.h"

uint8_t collision_check_tiles(const Entity *entity)
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
        right >= WORLD_TILES_WORLDMAP_MAP_TOTAL_X || bottom >= WORLD_TILES_WORLDMAP_MAP_TOTAL_Y) {
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

uint8_t collision_check_object(const Entity *entity, GameObject *objects)
{
    uint8_t i;
    // uint16_t left_col;
    // uint16_t right_col;
    // uint16_t top_row;
    // uint16_t bottom_row;
    int16_t object_left;
    int16_t object_right;
    int16_t object_top;
    int16_t object_bottom;

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
        right >= WORLD_TILES_WORLDMAP_MAP_TOTAL_X || bottom >= WORLD_TILES_WORLDMAP_MAP_TOTAL_Y) {
        return true;
    }

    for (i = 0; i < OBJECT_COUNT; i++)
    {
        if (!objects[i].collision)
            continue;

        object_left = objects[i].world_x + objects[i].hitbox.x;
        object_right = object_left + objects[i].hitbox.width - 1;
        object_top = objects[i].world_y + objects[i].hitbox.y;
        object_bottom = object_top + objects[i].hitbox.height - 1;

        if (left <= object_right &&
            right >= object_left &&
            top <= object_bottom &&
            bottom >= object_top)
        {
            return i;
        }
    }

    return 255;
}
