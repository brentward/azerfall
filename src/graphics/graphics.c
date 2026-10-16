#include "graphics.h"

bool sprite_is_on_screen(int16_t x, int16_t y)
{
    /* Mode 5 sprites are 16x16; include partially visible sprites. */
    return x > -16 && x < SCREEN_WIDTH &&
           y > -16 && y < SCREEN_HEIGHT;
}

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
