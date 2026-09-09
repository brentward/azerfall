#include "player.h"
#include "player_sprites.h"
#include "game.h"

Player player = {0};

void player_init(void) {
    int i;

    player.x = 160;
    player.y = 90;
    player.speed = 2;
    player.direction = DIR_DOWN;
    player.state = PLAYER_WALKING;
    player.animation_frame = 0;
    player.animation_timer = 0;

    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, x_pos_px, player.x);
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, y_pos_px, player.y);
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
    
    if (Input.up_pressed && !Input.right_pressed && !Input.left_pressed)
    {
        player.direction = DIR_UP;
        player.y -= player.speed;
        player.state = PLAYER_WALKING;

    }
    if (Input.down_pressed && !Input.right_pressed && !Input.left_pressed)
    {
        player.direction = DIR_DOWN;
        player.y += player.speed;
        player.state = PLAYER_WALKING;
    }
    if (Input.left_pressed && !Input.up_pressed && !Input.down_pressed)
    {
        player.direction = DIR_LEFT;
        player.x -= player.speed;
        player.state = PLAYER_WALKING;
    }
    if (Input.right_pressed && !Input.up_pressed && !Input.down_pressed)
    {
        player.direction = DIR_RIGHT;
        player.x += player.speed;
        player.state = PLAYER_WALKING;
    }
    if (Input.up_pressed && Input.left_pressed)
    {
        player.direction = DIR_UP_LEFT;
        move_speed = (player.speed + player.speed + player.speed) >> 2;
        player.x -= move_speed;
        player.y -= move_speed;
        player.state = PLAYER_WALKING;
    }
    if (Input.down_pressed && Input.left_pressed)
    {
        player.direction = DIR_DOWN_LEFT;
        move_speed = (player.speed + player.speed + player.speed) >> 2;
        player.x -= move_speed;
        player.y += move_speed;
        player.state = PLAYER_WALKING;
    }
    if (Input.up_pressed && Input.right_pressed)
    {
        player.direction = DIR_UP_RIGHT;
        move_speed = (player.speed + player.speed + player.speed) >> 2;
        player.x += move_speed;
        player.y -= move_speed;
        player.state = PLAYER_WALKING;
    }
    if (Input.down_pressed && Input.right_pressed)
    {
        player.direction = DIR_DOWN_RIGHT;
        move_speed = (player.speed + player.speed + player.speed) >> 2;
        player.x += move_speed;
        player.y += move_speed;
        player.state = PLAYER_WALKING;
    }
    player.animation_timer++;
    if (player.animation_timer >= 12) { // Example timer threshold
        player.animation_timer = 0;
        player.animation_frame = (player.animation_frame + 1) % 4; // Example animation frame update
    }

    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, x_pos_px, player.x);
    xram0_struct_set(PLAYER_SPRITE_CONFIG, vga_mode5_sprite_t, y_pos_px, player.y);
    player_animation_update();
}

void player_animation_update(void) {
    // Update player animation based on state and direction
    // This is a placeholder for actual animation logic
    int sprite_index;

    switch (player.state) {
        case PLAYER_IDLE:
            sprite_index = player.direction * 4;
            break;
        case PLAYER_WALKING:
            sprite_index = player.direction * 4 + player.animation_frame;
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

void player_draw(void) {
    // Draw player on the screen here
}
