#include "player.h"
#include "../../generated/player_sprites.h"
#include "game.h"
#include "entity.h"
#include "../input/input.h"

Player player = {0};

void player_init(void) {
    int i;

    player.entity.world_x = 100;
    player.entity.world_y = 100;
    player.entity.speed = 2;
    player.entity.direction = DIR_DOWN;
    player.state = PLAYER_WALKING;
    player.animation_frame = 0;
    player.animation_timer = 0;

    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X);
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y);
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, xram_sprite_ptr, PLAYER_SPRITES + PLAYER_WALK_DIR0_FRAME0);
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, palette_ptr, PLAYER_PALETTE);

    RIA.addr0 = PLAYER_SPRITES;
    for (i = 0; i < PLAYER_SPRITES_TOTAL_BYTES; i++)
    {
        RIA.rw0 = player_sprites[i];
    }
    // LENGTH is the number of sprite configs, not a byte size.
    xreg_vga_mode(5, 10, PLAYER_SPRITE_CONFIG, 1, 2);

}

void player_update(void) {
    int move_speed;
    player.state = PLAYER_IDLE;
    
    if (input_state.up_pressed && !input_state.right_pressed && !input_state.left_pressed)
    {
        player.entity.direction = DIR_UP;
        player.entity.world_y += player.entity.speed;
        player.state = PLAYER_WALKING;

    }
    if (input_state.down_pressed && !input_state.right_pressed && !input_state.left_pressed)
    {
        player.entity.direction = DIR_DOWN;
        player.entity.world_y -= player.entity.speed;
        player.state = PLAYER_WALKING;
    }
    if (input_state.left_pressed && !input_state.up_pressed && !input_state.down_pressed)
    {
        player.entity.direction = DIR_LEFT;
        player.entity.world_x += player.entity.speed;
        player.state = PLAYER_WALKING;
    }
    if (input_state.right_pressed && !input_state.up_pressed && !input_state.down_pressed)
    {
        player.entity.direction = DIR_RIGHT;
        player.entity.world_x -= player.entity.speed;
        player.state = PLAYER_WALKING;
    }
    if (input_state.up_pressed && input_state.left_pressed)
    {
        player.entity.direction = DIR_UP_LEFT;
        move_speed = (player.entity.speed + player.entity.speed + player.entity.speed) >> 2;
        player.entity.world_x += move_speed;
        player.entity.world_y += move_speed;
        player.state = PLAYER_WALKING;
    }
    if (input_state.down_pressed && input_state.left_pressed)
    {
        player.entity.direction = DIR_DOWN_LEFT;
        move_speed = (player.entity.speed + player.entity.speed + player.entity.speed) >> 2;
        player.entity.world_x += move_speed;
        player.entity.world_y -= move_speed;
        player.state = PLAYER_WALKING;
    }
    if (input_state.up_pressed && input_state.right_pressed)
    {
        player.entity.direction = DIR_UP_RIGHT;
        move_speed = (player.entity.speed + player.entity.speed + player.entity.speed) >> 2;
        player.entity.world_x -= move_speed;
        player.entity.world_y += move_speed;
        player.state = PLAYER_WALKING;
    }
    if (input_state.down_pressed && input_state.right_pressed)
    {
        player.entity.direction = DIR_DOWN_RIGHT;
        move_speed = (player.entity.speed + player.entity.speed + player.entity.speed) >> 2;
        player.entity.world_x -= move_speed;
        player.entity.world_y -= move_speed;
        player.state = PLAYER_WALKING;
    }
    player.animation_timer++;
    if (player.animation_timer >= 12) { // Example timer threshold
        player.animation_timer = 0;
        player.animation_frame = (player.animation_frame + 1) % 4; // Example animation frame update
    }

    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X);
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, x_pos_px, player.entity.world_x - PLAYER_SCREEN_X);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, y_pos_px, player.entity.world_y - PLAYER_SCREEN_Y);
    player_animation_update();
}

void player_animation_update(void) {
    // Update player animation based on state and direction
    // This is a placeholder for actual animation logic
    int sprite_index;

    switch (player.state) {
        case PLAYER_IDLE:
            sprite_index = player.entity.direction * 4;
            break;
        case PLAYER_WALKING:
            sprite_index = player.entity.direction * 4 + player.animation_frame;
            // Animation frame is updated in player_update based on timer
            break;
        case PLAYER_ATTACKING:
            // Handle attacking animation
            break;
        case PLAYER_DYING:
            // Handle dying animation
            break;
    }
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, xram_sprite_ptr, PLAYER_SPRITES + (sprite_index * BYTES_PER_SPRITE));
}
