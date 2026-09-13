#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rp6502.h>

#include "../input/input.h"
#include "../world/collision.h"
#include "../../generated/player_sprites.h"
#include "../../generated/world01_tiles.h"

#define PLAYER_SPRITES (BACKGROUND_TILES + WORLD01_TILES_TOTAL_BYTES)

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
    hitbox = &entity->hitbox;
    hitbox->x = 2;
    hitbox->y = 4;
    hitbox->width = 8;
    hitbox->height = 8;
    
    player->state = PLAYER_WALKING;
    player->animation_frame = 0;
    player->animation_timer = 0;
    player->sprite_xram_addr = PLAYER_SPRITES + PLAYER_WALK_DIR0_FRAME0;
}

void player_graphics_init(void)
{
    int i;

    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X);
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y);
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, xram_sprite_ptr, PLAYER_SPRITES + PLAYER_WALK_DIR0_FRAME0);
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, palette_ptr, PLAYER_PALETTE);

    RIA.addr0 = PLAYER_SPRITES;
    RIA.step0 = 1;
    for (i = 0; i < PLAYER_SPRITES_TOTAL_BYTES; i++)
    {
        RIA.rw0 = player_sprites[i];
    }
    // LENGTH is the number of sprite configs, not a byte size.
    // xreg_vga_mode(5, 10, PLAYER_SPRITE_CONFIG, 1, 2);
    if (xreg_vga_mode(5, 10, PLAYER_SPRITE_CONFIG, 1, 2) < 0)
    {
        perror("VGA sprite setup");
        exit(EXIT_FAILURE);
    }
}

void player_update(Player *player)
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

    if (player->state == PLAYER_WALKING && !collision_check_tiles(entity)) {
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

    player->animation_timer++;
    if (player->animation_timer >= 12) { // Example timer threshold
        player->animation_timer = 0;
        player->animation_frame = (player->animation_frame + 1) % 4; // Example animation frame update
    }

    switch (player->state) {
        case PLAYER_IDLE:
            sprite_index = entity->direction * 4;
            break;
        case PLAYER_WALKING:
            sprite_index = entity->direction * 4 + player->animation_frame;
            // Animation frame is updated in player_update based on timer
            break;
        case PLAYER_ATTACKING:
            // Handle attacking animation
            break;
        case PLAYER_DYING:
            // Handle dying animation
            break;
    }
    player->sprite_xram_addr = PLAYER_SPRITES + sprite_index * BYTES_PER_SPRITE;
}

void player_draw(Player *player)
{
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, xram_sprite_ptr, player->sprite_xram_addr);
}
