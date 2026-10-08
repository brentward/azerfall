#include "event.h"

#include "../input/input.h"
#include "../entity/entity.h"
#include "ui.h"

static bool pit_armed;

void event_init(void)
{
    pit_armed = true;
}

static bool event_hit(int16_t x, int16_t y, player_t *player, direction_t required_direction)
{
    entity_t *entity = &player->entity;  

    int16_t event_right;
    int16_t event_bottom;

    int16_t left = entity->world_x + entity->hitbox.x;
    int16_t right = left + entity->hitbox.width - 1;
    int16_t top = entity->world_y + entity->hitbox.y;
    int16_t bottom = top + entity->hitbox.height - 1;

    if (!(entity->direction == required_direction || required_direction == DIR_ANY))
        return false;

    /* Coordinates identify the top-left of a complete map tile. */
    event_right = x + TILE_SIZE - 1;

    if (left > event_right || right < x)
        return false;

    event_bottom = y + TILE_SIZE - 1;

    if (top <= event_bottom && bottom >= y)
        return true;
    
    return false;

}

static void event_damage_pit(game_state_t game_state, player_t *player)
{
    entity_t *entity = &player->entity;

    game_state_set(game_state);
    ui_show_dialogue("You fell into a pit!");
    player_damage(player, 1);

}

static void event_healing_pool(game_state_t game_state, player_t *player)
{
    entity_t *entity = &player->entity;

    if (input_state.interact_pressed)
    {
        input_state.interact_pressed = false;
        game_state_set(game_state);
        ui_show_dialogue("You drink the water.\nYour life has been\nrecovered!");
        entity->life = entity->max_life;
    }
    
}   

void event_check(player_t *player)
{
    if (!event_hit(432, 256, player, DIR_ANY))
        pit_armed = true;
    if (pit_armed && event_hit(432, 256, player, DIR_RIGHT))
    {
        pit_armed = false;
        event_damage_pit(GAME_STATE_DIALOGUE, player);
        return;
    }
    if (event_hit(368, 192, player, DIR_UP))
        event_healing_pool(GAME_STATE_DIALOGUE, player);
}
