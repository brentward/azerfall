#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rp6502.h>

#include "../xram_layout.h"
#include "../input/input.h"
#include "../world/collision.h"

static void player_animation_update(Player *player);

void player_init(Player *player)
{
    Entity *entity;
    HitBox *hitbox;
    memset(player, 0, sizeof *player);

    entity = &player->entity;
    entity->world_x = 368;
    entity->world_y = 336;
    entity->speed = 1;
    entity->direction = DIR_DOWN;

    entity->animation_frame = 0;
    entity->animation_timer = 0;
    entity->xram_sprite_ptr = XRAM_PLAYER_IMAGES + PLAYER_WALK_DIR0_FRAME0 * BYTES_PER_SPRITE;

    hitbox = &entity->hitbox;
    hitbox->x = 2;
    hitbox->y = 4;
    hitbox->width = 8;
    hitbox->height = 8;
    
    player->state = PLAYER_WALKING;
    player->screen_org_x = PLAYER_SCREEN_X - entity->world_x;
    player->screen_org_y = PLAYER_SCREEN_Y - entity->world_y;
}

void player_graphics_init(void)
{
    int i;

    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X);
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, xram_sprite_ptr, XRAM_PLAYER_IMAGES + PLAYER_WALK_DIR0_FRAME0);
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, palette_ptr, PLAYER_PALETTE);

    RIA.addr0 = XRAM_PLAYER_IMAGES;
    RIA.step0 = 1;
    for (i = 0; i < PLAYER_SPRITES_TOTAL_BYTES; i++)
    {
        RIA.rw0 = player_sprites[i];
    }
    // LENGTH is the number of sprite configs, not a byte size.
    // xreg_vga_mode(5, 10, PLAYER_SPRITE_CONFIG, 1, 2);
    if (xreg_vga_mode(5, 10, XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), 8, 2) < 0)
    {
        perror("VGA sprite setup");
        exit(EXIT_FAILURE);
    }
}

void player_update(Player *player, GameObject objects[OBJECT_COUNT])
{
    Entity *entity = &player->entity;
    player->state = PLAYER_IDLE;

    
    if (input_state.up_pressed && !input_state.down_pressed && !input_state.left_pressed && !input_state.right_pressed)
    {
        entity->direction = DIR_UP;
        player->state = PLAYER_WALKING;
    }
    if (!input_state.up_pressed && input_state.down_pressed && !input_state.left_pressed && !input_state.right_pressed)
    {
        entity->direction = DIR_DOWN;
        player->state = PLAYER_WALKING;
    }
    if (!input_state.up_pressed && !input_state.down_pressed && input_state.left_pressed && !input_state.right_pressed)
    {
        entity->direction = DIR_LEFT;
        player->state = PLAYER_WALKING;
    }
    if (!input_state.up_pressed && !input_state.down_pressed && !input_state.left_pressed && input_state.right_pressed)
    {
        entity->direction = DIR_RIGHT;
        player->state = PLAYER_WALKING;
    }
    if (input_state.up_pressed && !input_state.down_pressed && input_state.left_pressed && !input_state.right_pressed)
    {
        entity->direction = DIR_UP_LEFT;
        player->state = PLAYER_WALKING;
    }
    if (!input_state.up_pressed && input_state.down_pressed && input_state.left_pressed && !input_state.right_pressed)
    {
        entity->direction = DIR_DOWN_LEFT;
        player->state = PLAYER_WALKING;
    }
    if (input_state.up_pressed && !input_state.down_pressed && !input_state.left_pressed && input_state.right_pressed)
    {
        entity->direction = DIR_UP_RIGHT;
        player->state = PLAYER_WALKING;
    }
    if (!input_state.up_pressed && input_state.down_pressed && !input_state.left_pressed && input_state.right_pressed)
    {
        entity->direction = DIR_DOWN_RIGHT;
        player->state = PLAYER_WALKING;
    }

    if (player->state == PLAYER_WALKING && !collision_check_tiles(entity) && collision_check_object(entity, objects) == 255) {
        switch (entity->direction) {
        case DIR_UP:
            entity->world_y -= entity->speed;
            break;

        case DIR_DOWN:
            entity->world_y += entity->speed;
            break;

        case DIR_LEFT:
            entity->world_x -= entity->speed;
            break;

        case DIR_RIGHT:
            entity->world_x += entity->speed;
            break;

        case DIR_UP_LEFT:
            entity->world_x -= entity->speed;
            entity->world_y -= entity->speed;
            break;

        case DIR_DOWN_LEFT:
            entity->world_x -= entity->speed;
            entity->world_y += entity->speed;
            break;

        case DIR_UP_RIGHT:
            entity->world_x += entity->speed;
            entity->world_y -= entity->speed;
            break;

        case DIR_DOWN_RIGHT:
            entity->world_x += entity->speed;
            entity->world_y += entity->speed;
            break;
        } 
    }
    
    player_animation_update(player);
}

static void player_animation_update(Player *player)
{
    // Update player animation based on state and direction
    // This is a placeholder for actual animation logic
    Entity *entity = &player->entity;
    int sprite_index = entity->direction * 4;

    entity->animation_timer++;
    if (entity->animation_timer >= 12) { // Example timer threshold
        entity->animation_timer = 0;
        entity->animation_frame = (entity->animation_frame + 1) % 4; // Example animation frame update
    }

    switch (player->state) {
        case PLAYER_IDLE:
            sprite_index = entity->direction * 4;
            break;
        case PLAYER_WALKING:
            sprite_index = entity->direction * 4 + entity->animation_frame;
            // Animation frame is updated in player_update based on timer
            break;
        case PLAYER_ATTACKING:
            // Handle attacking animation
            break;
        case PLAYER_DYING:
            // Handle dying animation
            break;
    }
    entity->xram_sprite_ptr =  XRAM_PLAYER_IMAGES + sprite_index * BYTES_PER_SPRITE;
}

void player_draw(Player *player)
{
    Entity *entity = &player->entity;

    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, xram_sprite_ptr, entity->xram_sprite_ptr);
}
