#include "monster.h"

#include <string.h>

#include "../xram.h"
#include "entity.h"
#include "player.h"
#include "../graphics/graphics.h"

void monster_sprite_init(void)
{
    const uint16_t *palette = monster_sprites_palette;
    int i;
    const uint8_t *pixels = monster_sprites;

    RIA.addr0 = XRAM_MONSTER_IMAGES;
    RIA.step0 = 1;
    for (i = 0; i < MONSTER_SPRITES_TOTAL_BYTES; i++, pixels++)
    {
        RIA.rw0 = *pixels;
    }

    RIA.addr0 = XRAM_MONSTER_PALETTE;
    for (i = 0; i < MONSTER_SPRITES_PALETTE_COUNT; i++, palette++)
    {
        RIA.rw0 = (uint8_t)*palette;
        RIA.rw0 = (uint8_t)(*palette >> 8);
    }
    
}


void monster_greenslime_init(entity_t *greenslime, player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y)
{
    entity_t *entity = &player->entity;
    screen_position_t screen_position;
    hitbox_t *hitbox;
    entity_type_t *type;

    memset(greenslime, 0, sizeof *greenslime);
    hitbox = &greenslime->hitbox;
    type = &greenslime->type;
    hitbox->x = 3;
    hitbox->y = 8;
    hitbox->width = 10;
    hitbox->height = 7;
    greenslime->type = ENTITY_MONSTER_GREENSLIME;
    greenslime->world_x = world_x;
    greenslime->world_y = world_y;
    greenslime->speed = 1;
    greenslime->max_life = 4;
    greenslime->life = greenslime->max_life;
    greenslime->attack = 1;
    greenslime->xram_sprite_ptr = XRAM_MONSTER_IMAGES + GREENSLIME_FRAME0 * BYTES_PER_SPRITE;
    screen_position = get_screen_position_entity(greenslime, player);
    greenslime->screen_x = screen_position.screen_x;
    greenslime->screen_y = screen_position.screen_y;

    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, x_pos_px, greenslime->screen_x);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, y_pos_px, greenslime->screen_y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, xram_sprite_ptr, greenslime->xram_sprite_ptr);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, palette_ptr, XRAM_MONSTER_PALETTE);
}

void monster_greenslime_update(entity_t *greenslime)
{
    int i;

    greenslime->action_lock_counter++;
    if (greenslime->action_lock_counter > 120)
    {
        i = rand();

        if (i <= DIR_UP_DIVIDE)
        {
            greenslime->direction = DIR_UP;
        }
        if (i > DIR_UP_DIVIDE && i <= DIR_DOWN_DIVIDE)
        {
            greenslime->direction = DIR_DOWN;
        }
        if (i > DIR_DOWN_DIVIDE && i <= DIR_LEFT_DIVIDE)
        {
            greenslime->direction = DIR_LEFT;
        }
        if (i > DIR_LEFT_DIVIDE && i <= DIR_RIGHT_DIVIDE)
        {
            greenslime->direction = DIR_RIGHT;
        }
        greenslime->action_lock_counter = 0;
    }
}
