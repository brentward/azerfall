#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rp6502.h>

#include "../xram.h"
#include "../input/input.h"
#include "../world/collision.h"
#include "../audio/sound.h"
#include "ui.h"

static void player_animation_update(player_t *player);
static void player_pickup_object(player_t *player, object_t *objects, uint8_t index);


void player_init(player_t *player)
{
    entity_t *entity;
    hitbox_t *hitbox;
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
    // // testing
    // player->key_count = 10;
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
    if (xreg_vga_mode5(MODE5_4BPP | MODE5_16X16, XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), 8, VGA_PLANE_SPRITES) < 0)
    {
        perror("VGA sprite setup");
        exit(EXIT_FAILURE);
    }
}

void player_update(player_t *player, object_t objects[OBJECT_COUNT])
{
    entity_t *entity = &player->entity;
    uint8_t index_x;
    uint8_t index_y;
    bool blocked_x;
    int16_t dx;
    int16_t dy;

    // Resolve each axis independently so opposing directions cancel.
    int move_x = input_state.right_pressed - input_state.left_pressed;
    int move_y = input_state.down_pressed - input_state.up_pressed;

    player->state = PLAYER_IDLE;
    if (move_x != 0 || move_y != 0)
    {
        player->state = PLAYER_WALKING;
        if (move_y < 0)
        {
            entity->direction = move_x < 0 ? DIR_UP_LEFT :
                                move_x > 0 ? DIR_UP_RIGHT : DIR_UP;
        }
        else if (move_y > 0)
        {
            entity->direction = move_x < 0 ? DIR_DOWN_LEFT :
                                move_x > 0 ? DIR_DOWN_RIGHT : DIR_DOWN;
        }
        else
        {
            entity->direction = move_x < 0 ? DIR_LEFT : DIR_RIGHT;
        }
    }
    dx = move_x * entity->speed;
    dy = move_y * entity->speed;

    // Move X first, then test Y from the resulting position.
    // Keep facing the input direction while sliding along an obstacle.
    entity->collision_on = false;
    collision_check_tiles(entity, dx, 0);
    index_x = collision_check_object(entity, objects, dx, 0);
    blocked_x = entity->collision_on;
    if (!blocked_x)
        entity->world_x += dx;

    entity->collision_on = false;
    collision_check_tiles(entity, 0, dy);
    index_y = collision_check_object(entity, objects, 0, dy);
    if (!entity->collision_on)
        entity->world_y += dy;
    entity->collision_on = entity->collision_on || blocked_x;

    // Interact even when movement is blocked, but only once per object.
    player_pickup_object(player, objects, index_x);
    if (index_y != index_x)
        player_pickup_object(player, objects, index_y);

    player_animation_update(player);
}

static void player_animation_update(player_t *player)
{
    // Update player animation based on state and direction
    // This is a placeholder for actual animation logic
    entity_t *entity = &player->entity;
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

static void player_pickup_object(player_t *player, object_t *objects, uint8_t index)
{
    if (index != 255)
    {
        switch (objects[index].type) {
            case OBJECT_CHEST:
                if (objects[index].state == CHEST_CLOSED)
                {
                    objects[index].state = CHEST_OPEN;
                }
    
                break;
            case OBJECT_DOOR:
                if (objects[index].state == DOOR_CLOSED)
                {
                    if (player->key_count > 0)
                    {
                        player->key_count--;
                        printf("Key: %u\n", player->key_count);
                        objects[index].collision = false;
                        sound_play(SFX_DOOR);
                        ui_show_message("You opened a door!");
                        objects[index].state = DOOR_OPENED;
                    } else
                    {
                        ui_show_message("You need a key!");
                    }
                }
                break;
            case OBJECT_KEY:
                player->key_count++;
                sound_play(SFX_PICKUP);
                ui_show_message("You got a key!");
                printf("Key: %u\n", player->key_count);
                objects[index].world_x = -500;
                objects[index].world_y = -500;
                break;
        }

    }
}

void player_draw(player_t *player)
{
    entity_t *entity = &player->entity;

    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, xram_sprite_ptr, entity->xram_sprite_ptr);
}
