#include "graphics.h"

screen_position_t get_screen_position_object(object_t *obj, player_t *player)
{
    screen_position_t screen_position;
    entity_t *entity = &player->entity;

    screen_position.screen_x = obj->world_x - entity->world_x + PLAYER_SCREEN_X;
    screen_position.screen_y = obj->world_y - entity->world_y + PLAYER_SCREEN_Y;

    return screen_position;
}

screen_position_t get_screen_position_entity(entity_t *entity, player_t *player)
{
    screen_position_t screen_position;
    entity_t *player_entity = &player->entity;

    screen_position.screen_x = entity->world_x - player_entity->world_x + PLAYER_SCREEN_X;
    screen_position.screen_y = entity->world_y - player_entity->world_y + PLAYER_SCREEN_Y;

    return screen_position;
}
