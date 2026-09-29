#include "collision.h"

#include "map.h"
#include "../game/game.h"
#include "../../generated/world_tiles_worldmap_map.h"

void collision_check_tiles(entity_t *entity, int16_t dx, int16_t dy)
{
    uint16_t left_col;
    uint16_t right_col;
    uint16_t top_row;
    uint16_t bottom_row;

    int16_t left = entity->world_x + entity->hitbox.x + dx;

    int16_t right = left + entity->hitbox.width - 1;

    int16_t top = entity->world_y + entity->hitbox.y + dy;

    int16_t bottom = top + entity->hitbox.height - 1;

    if (left < 0 || top < 0 ||
        right >= WORLD_TILES_WORLDMAP_MAP_TOTAL_X || bottom >= WORLD_TILES_WORLDMAP_MAP_TOTAL_Y) {
        entity->collision_on = true;
        return;
    }

    left_col = left / TILE_SIZE;
    right_col = right / TILE_SIZE;
    top_row = top / TILE_SIZE;
    bottom_row = bottom / TILE_SIZE;

    entity->collision_on = 
        (map_tile_is_solid(left_col, top_row) ||
        map_tile_is_solid(right_col, top_row) ||
        map_tile_is_solid(left_col, bottom_row) ||
        map_tile_is_solid(right_col, bottom_row));

}

uint8_t collision_check_object(entity_t *entity, object_t *objects, int16_t dx, int16_t dy)
{
    uint8_t i;
    uint8_t index = 255;
    int16_t object_left;
    int16_t object_right;
    int16_t object_top;
    int16_t object_bottom;

    int16_t left = entity->world_x + entity->hitbox.x + dx;

    int16_t right = left + entity->hitbox.width - 1;

    int16_t top = entity->world_y + entity->hitbox.y + dy;

    int16_t bottom = top + entity->hitbox.height - 1;

    if (left < 0 || top < 0 ||
        right >= WORLD_TILES_WORLDMAP_MAP_TOTAL_X || bottom >= WORLD_TILES_WORLDMAP_MAP_TOTAL_Y) {
        entity->collision_on = true;
    }

    for (i = 0; i < OBJECT_COUNT; i++)
    {
        object_left = objects[i].world_x + objects[i].hitbox.x;
        object_right = object_left + objects[i].hitbox.width - 1;
        object_top = objects[i].world_y + objects[i].hitbox.y;
        object_bottom = object_top + objects[i].hitbox.height - 1;

        if (left <= object_right &&
            right >= object_left &&
            top <= object_bottom &&
            bottom >= object_top)
        {
            if (objects[i].collision)
            {
                entity->collision_on = true;
            }
            // Keep checking: a pickup must not hide another solid object.
            if (index == 255)
                index = i;
        }
    }

    return index;
}
