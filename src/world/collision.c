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
    object_t *object = objects;
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

    for (i = 0; i < OBJECT_COUNT; i++, object++)
    {
        /* Empty slots have no hitbox. */
        if (object->hitbox.width == 0 ||
            object->hitbox.height == 0)
            continue;
        object_left = object->world_x + object->hitbox.x;
        object_right = object_left + object->hitbox.width - 1;

        /* No horizontal overlap: move to the next object. */
        if (left > object_right || right < object_left)
            continue;

        /* Only calculate vertical bounds when horizontal overlap exists. */
        object_top = object->world_y + object->hitbox.y;
        object_bottom = object_top + object->hitbox.height - 1;

        if (top <= object_bottom && bottom >= object_top)
        {
            if (object->collision)
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

uint8_t collision_check_npcs(entity_t *entity, npc_t *npcs, int16_t dx, int16_t dy)
{
    npc_t *target = npcs;
    uint8_t i;
    uint8_t index = 255;
    int16_t target_left;
    int16_t target_right;
    int16_t target_top;
    int16_t target_bottom;
    entity_t *target_entity;


    int16_t left = entity->world_x + entity->hitbox.x + dx;

    int16_t right = left + entity->hitbox.width - 1;

    int16_t top = entity->world_y + entity->hitbox.y + dy;

    int16_t bottom = top + entity->hitbox.height - 1;

    for (i = 0; i < NPC_COUNT; i++, target++)
    {
        target_entity = &target->entity;

        /* Empty slots have no hitbox. Compare addresses to skip only ourselves. */
        if (target_entity == entity || target_entity->hitbox.width == 0 ||
            target_entity->hitbox.height == 0)
            continue;

        target_left = target_entity->world_x + target_entity->hitbox.x;
        target_right = target_left + target_entity->hitbox.width - 1;

        /* No horizontal overlap: move to the next object. */
        if (left > target_right || right < target_left)
            continue;

        /* Only calculate vertical bounds when horizontal overlap exists. */
        target_top = target_entity->world_y + target_entity->hitbox.y;
        target_bottom = target_top + target_entity->hitbox.height - 1;

        if (top <= target_bottom && bottom >= target_top)
        {
            entity->collision_on = true;
            return i;
        }
    }

    return index;
}

uint8_t collision_check_monsters(entity_t *entity, entity_t *monsters, int16_t dx, int16_t dy)
{
    entity_t *monster = monsters;
    uint8_t i;
    uint8_t index = 255;
    int16_t monster_left;
    int16_t monster_right;
    int16_t monster_top;
    int16_t monster_bottom;

    int16_t left = entity->world_x + entity->hitbox.x + dx;
    int16_t right = left + entity->hitbox.width - 1;
    int16_t top = entity->world_y + entity->hitbox.y + dy;
    int16_t bottom = top + entity->hitbox.height - 1;

    for (i = 0; i < MONSTER_COUNT; i++, monster++)
    {


        /* Empty slots have no hitbox. Compare addresses to skip only ourselves. */
        if (monster == entity || monster ->hitbox.width == 0 ||
            monster->hitbox.height == 0)
            continue;

        monster_left = monster->world_x + monster->hitbox.x;
        monster_right = monster_left + monster->hitbox.width - 1;

        /* No horizontal overlap: move to the next object. */
        if (left > monster_right || right < monster_left)
            continue;

        /* Only calculate vertical bounds when horizontal overlap exists. */
        monster_top = monster->world_y + monster->hitbox.y;
        monster_bottom = monster_top + monster->hitbox.height - 1;

        if (top <= monster_bottom && bottom >= monster_top)
        {
            entity->collision_on = true;
            return i;
        }
    }

    return index;
}


void collision_check_player(entity_t *entity, player_t *player, int16_t dx, int16_t dy)
{
    entity_t *player_entity = &player->entity;
    int16_t target_left;
    int16_t target_right;
    int16_t target_top;
    int16_t target_bottom;

    int16_t left = entity->world_x + entity->hitbox.x + dx;

    int16_t right = left + entity->hitbox.width - 1;

    int16_t top = entity->world_y + entity->hitbox.y + dy;

    int16_t bottom = top + entity->hitbox.height - 1;

    target_left = player_entity->world_x + player_entity->hitbox.x;
    target_right = target_left + player_entity->hitbox.width - 1;
    target_top = player_entity->world_y + player_entity->hitbox.y;
    target_bottom = target_top + player_entity->hitbox.height - 1;

    if (left <= target_right &&
        right >= target_left &&
        top <= target_bottom &&
        bottom >= target_top)
    {
        entity->collision_on = true;
    }
}
