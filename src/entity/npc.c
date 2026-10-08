#include "npc.h"

#include <stdlib.h>
#include <string.h>

#include "../xram.h"
#include "entity.h"
#include "player.h"
#include "../graphics/graphics.h"
#include "../game/ui.h"


void npc_sprite_init(void)
{
    const uint16_t *palette = npc_sprites_palette;
    int i;
    const uint8_t *pixels = npc_sprites;

    RIA.addr0 = XRAM_NPC_IMAGES;
    RIA.step0 = 1;
    for (i = 0; i < NPC_SPRITES_TOTAL_BYTES; i++, pixels++)
    {
        RIA.rw0 = *pixels;
    }

    RIA.addr0 = XRAM_NPC_PALETTE;
    for (i = 0; i < NPC_SPRITES_PALETTE_COUNT; i++, palette++)
    {
        RIA.rw0 = (uint8_t)*palette;
        RIA.rw0 = (uint8_t)(*palette >> 8);
    }
    
}

void npc_oldman_init(entity_t *oldman, player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y)
{
    entity_t *entity = &player->entity;
    screen_position_t screen_position;
    hitbox_t *hitbox;
    entity_type_t *type;

    memset(oldman, 0, sizeof *oldman);
    hitbox = &oldman->hitbox;
    type = &oldman->type;
    hitbox->x = 0;
    hitbox->y = 0;
    hitbox->width = 16;
    hitbox->height = 16;
    oldman->type = ENTITY_NPC_OLDMAN;
    oldman->world_x = world_x;
    oldman->world_y = world_y;
    oldman->speed = 1;
    oldman->xram_sprite_ptr = XRAM_NPC_IMAGES + OLDMAN_DIR0_FRAME0 * BYTES_PER_SPRITE;
    screen_position = get_screen_position_entity(oldman, player);
    oldman->screen_x = screen_position.screen_x;
    oldman->screen_y = screen_position.screen_y;

    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, x_pos_px, oldman->screen_x);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, y_pos_px, oldman->screen_y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, xram_sprite_ptr, oldman->xram_sprite_ptr);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, palette_ptr, XRAM_NPC_PALETTE);
}

void npc_merchant_init(entity_t *merchant, player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y)
{
    entity_t *entity = &player->entity;
    screen_position_t screen_position;
    hitbox_t *hitbox;
    entity_type_t *type;

    memset(merchant, 0, sizeof *merchant);
    hitbox = &merchant->hitbox;
    type = &merchant->type;
    hitbox->x = 0;
    hitbox->y = 0;
    hitbox->width = 16;
    hitbox->height = 16;
    merchant->type = ENTITY_NPC_MERCHANT;
    merchant->world_x = world_x;
    merchant->world_y = world_y;
    merchant->xram_sprite_ptr = XRAM_NPC_IMAGES + MERCHANT_DIR0_FRAME0 * BYTES_PER_SPRITE;
    screen_position = get_screen_position_entity(merchant, player);
    merchant->screen_x = screen_position.screen_x;
    merchant->screen_y = screen_position.screen_y;

    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, x_pos_px, merchant->screen_x);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, y_pos_px, merchant->screen_y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, xram_sprite_ptr, merchant->xram_sprite_ptr);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, palette_ptr, XRAM_NPC_PALETTE);
}

void npc_oldman_update(entity_t *oldman)
{
    int i;

    oldman->action_lock_counter++;
    if (oldman->action_lock_counter > 120)
    {
        i = rand();

        if (i <= DIR_UP_DIVIDE)
        {
            oldman->direction = DIR_UP;
        }
        if (i > DIR_UP_DIVIDE && i <= DIR_DOWN_DIVIDE)
        {
            oldman->direction = DIR_DOWN;
        }
        if (i > DIR_DOWN_DIVIDE && i <= DIR_LEFT_DIVIDE)
        {
            oldman->direction = DIR_LEFT;
        }
        if (i > DIR_LEFT_DIVIDE && i <= DIR_RIGHT_DIVIDE)
        {
            oldman->direction = DIR_RIGHT;
        }
        oldman->action_lock_counter = 0;
    }
}

void npc_oldman_set_dialogue(npc_t *oldman, char *line1, char *line2, char *line3, char *line4)
{
    oldman->dialogue_lines[0] = line1;
    oldman->dialogue_lines[1] = line2;
    oldman->dialogue_lines[2] = line3;
    oldman->dialogue_lines[3] = line4;
}

void npc_speak(npc_t *npc, player_t *player)
{
    char *line = npc->dialogue_lines[npc->dialogue_index];
    entity_t *entity = &npc->entity;
    entity_t *player_entity = &player->entity;

    if (line != NULL)
    {
        ui_show_dialogue(line);
        npc->dialogue_index++;
        if (npc->dialogue_index >= 4 || npc->dialogue_lines[npc->dialogue_index] == NULL)
        {
            npc->dialogue_index = 0;
        }
        switch (player_entity->direction)
        {
            case DIR_UP:
                entity->direction = DIR_DOWN;
                break;
            case DIR_DOWN:
                entity->direction = DIR_UP;
                // Handle down direction
                break;
            // case DIR_UP_LEFT:
            // case DIR_DOWN_LEFT:
            case DIR_LEFT:
                entity->direction = DIR_RIGHT;
                // Handle left direction
                break;
            // case DIR_DOWN_RIGHT:
            // case DIR_UP_RIGHT:
            case DIR_RIGHT:
                entity->direction = DIR_LEFT;
                // Handle right direction
                break;
        }
    }
}

void npc_merchant_update(entity_t *merchant)
{
    
}
