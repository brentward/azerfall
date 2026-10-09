#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rp6502.h>

#include "../input/input.h"
#include "../world/collision.h"
#include "../audio/sound.h"
#include "../game/ui.h"
#include "../game/event.h"
#include "entity.h"

static void player_animation_update(player_t *player);
static void player_pickup_object(player_t *player, object_t *objects, uint8_t index);
static void player_interact_npc(player_t *player, npc_t *npcs, uint8_t index);
static void player_interact_monster(player_t *player, entity_t *monsters, uint8_t index);
static void player_interact_facing(player_t *player, object_t *objects, npc_t *npcs);
static void player_attack_monster(player_t *player, entity_t *monsters, uint8_t index);

void player_init(player_t *player)
{
    entity_t *entity;
    hitbox_t *hitbox;
    hitbox_t *attack_hitbox;

    memset(player, 0, sizeof *player);

    entity = &player->entity;
    entity->world_x = 368;
    entity->world_y = 336;
    entity->speed = 1;
    entity->direction = DIR_DOWN;
    entity->state = ENTITY_WALKING;
    entity->max_life = 12;
    entity->life = entity->max_life;
    entity->invincible = false;
    entity->attack = 1;



    entity->animation_frame = 0;
    entity->animation_timer = 0;
    entity->xram_sprite_ptr = XRAM_PLAYER_IMAGES + PLAYER_DIR0_FRAME0 * BYTES_PER_SPRITE;
    entity->xram_weapon_sprite_ptr = XRAM_PLAYER_IMAGES + PLAYER_EMPTY_FRAME0 * BYTES_PER_SPRITE;

    entity->type = ENTITY_PLAYER;


    hitbox = &entity->hitbox;
    attack_hitbox = &entity->attack_hitbox;
    hitbox->x = 2;
    hitbox->y = 5;
    hitbox->width = 10;
    hitbox->height = 10;
    
    player->screen_org_x = PLAYER_SCREEN_X - entity->world_x;
    player->screen_org_y = PLAYER_SCREEN_Y - entity->world_y;
    // // testing
    // player->key_count = 10;
}

void player_graphics_init(void)
{
    int i;
    const uint8_t *pixels = player_sprites;
    const uint16_t *palette = player_sprites_palette;


    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X);
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, xram_sprite_ptr, XRAM_PLAYER_IMAGES + PLAYER_DIR0_FRAME0 * BYTES_PER_SPRITE);
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), vga_mode5_sprite_t, palette_ptr, XRAM_PLAYER_PALETTE);

    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X);
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y + PLAYER_SPRITES_HEIGHT);
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, xram_sprite_ptr, XRAM_PLAYER_IMAGES + PLAYER_EMPTY_FRAME0 * BYTES_PER_SPRITE);
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, palette_ptr, XRAM_PLAYER_PALETTE);


    RIA.addr0 = XRAM_PLAYER_IMAGES;
    RIA.step0 = 1;
    for (i = 0; i < PLAYER_SPRITES_TOTAL_BYTES; i++, pixels++)
    {
        RIA.rw0 = *pixels;
    }

    RIA.addr0 = XRAM_PLAYER_PALETTE;
    for (i = 0; i < PLAYER_SPRITES_PALETTE_COUNT; i++, palette++)
    {
        RIA.rw0 = (uint8_t)*palette;
        RIA.rw0 = (uint8_t)(*palette >> 8);
    }
}

void player_update(player_t *player, object_t objects[OBJECT_COUNT], npc_t npcs[NPC_COUNT], entity_t monsters[MONSTER_COUNT])
{
    entity_t *entity = &player->entity;
    uint8_t object_index_x = 255;
    uint8_t object_index_y = 255;
    uint8_t npc_index_x = 255;
    uint8_t npc_index_y = 255;
    uint8_t monster_index_x = 255;
    uint8_t monster_index_y = 255;

    bool blocked_x;
    int16_t dx;
    int16_t dy;

    // Resolve each axis independently so opposing directions cancel.
    int move_x = input_state.right_pressed - input_state.left_pressed;
    int move_y = input_state.down_pressed - input_state.up_pressed;

    switch (entity->state)
    {
        case ENTITY_IDLE:
        case ENTITY_WALKING:
            entity->state = ENTITY_IDLE;
            if (move_x != 0 || move_y != 0)
            {
                entity->state = ENTITY_WALKING;
                // Only cardinal input changes facing; diagonals keep the last facing.
                if (move_x == 0)
                    entity->direction = move_y < 0 ? DIR_UP : DIR_DOWN;
                else if (move_y == 0)
                    entity->direction = move_x < 0 ? DIR_LEFT : DIR_RIGHT;
            }
            dx = move_x * entity->speed;
            dy = move_y * entity->speed;

            entity->collision_on = false;

            if (dx != 0 && dy != 0)
            {
            
                // Move X first, then test Y from the resulting position.
                // Keep the last cardinal facing while sliding along an obstacle.
                collision_check_tiles(entity, dx, 0);
                if (!entity->collision_on)
                    object_index_x = collision_check_object(entity, objects, dx, 0);
                if (!entity->collision_on)
                    npc_index_x = collision_check_npcs(entity, npcs, dx, 0);
                if (!entity->collision_on)
                    monster_index_x = collision_check_monsters(entity, monsters, dx, 0);
                blocked_x = entity->collision_on;
                if (!blocked_x)
                    entity->world_x += dx;

                entity->collision_on = false;
                collision_check_tiles(entity, 0, dy);
                if (!entity->collision_on)
                    object_index_y = collision_check_object(entity, objects, 0, dy);
                if (!entity->collision_on)
                    npc_index_y = collision_check_npcs(entity, npcs, 0, dy);
                if (!entity->collision_on)
                    monster_index_y = collision_check_monsters(entity, monsters, 0, dy);
                
                if (!entity->collision_on)
                    entity->world_y += dy;
                entity->collision_on = entity->collision_on || blocked_x;

                // Interact even when movement is blocked, but only once per object.
                player_pickup_object(player, objects, object_index_x);
                if (object_index_y != object_index_x)
                    player_pickup_object(player, objects, object_index_y);
                
                player_interact_npc(player, npcs, npc_index_x);

                if (npc_index_y != npc_index_x)
                    player_interact_npc(player, npcs, npc_index_y);

                player_interact_monster(player, monsters, monster_index_x);

                if (monster_index_y != monster_index_x)
                    player_interact_monster(player, monsters, monster_index_y);

            } else 
            {
                collision_check_tiles(entity, dx, dy);
                if (!entity->collision_on)
                    object_index_x = collision_check_object(entity, objects, dx, dy);
                if (!entity->collision_on)
                    npc_index_x = collision_check_npcs(entity, npcs, dx, dy);
                if (!entity->collision_on)
                    monster_index_x = collision_check_monsters(entity, monsters, dx, dy);

                player_pickup_object(player, objects, object_index_x);
                player_interact_npc(player, npcs, npc_index_x);
                player_interact_monster(player, monsters, monster_index_x);

                if (!entity->collision_on)
                {
                    entity->world_x += dx;
                    entity->world_y += dy;
                }
            }


            if (input_state.interact_pressed)
                player_interact_facing(player, objects, npcs);
            if (input_state.attack_pressed)
            {
                entity->attacking = true;
                entity->state = ENTITY_ATTACKING;
                entity->action_lock_counter = 0;
                entity->animation_frame = 0;
                entity->animation_timer = 0;
            }
            event_check(player);
            break;
        case ENTITY_ATTACKING:
            player_attacking(player, monsters);

    }


    if (entity->invincible)
    {
        if (++entity->invincible_counter >= 60)
        {
            entity->invincible = false;
            entity->invincible_counter = 0;
        }
    }
    // input_state.interact_pressed = false; // consume pressed interactions
    player_animation_update(player);
}

void player_attacking(player_t *player,  entity_t monsters[MONSTER_COUNT])
{
    entity_t *entity = &player->entity;
    hitbox_t *hitbox = &entity->hitbox;
    int16_t screen_x;
    int16_t screen_y;
    int16_t curent_world_x = entity->world_x;
    int16_t current_world_y = entity->world_y;
    uint8_t player_hitbox_x = hitbox->x;
    uint8_t player_hitbox_y = hitbox->y;
    uint8_t player_hitbox_width = hitbox->width;
    uint8_t player_hitbox_height = hitbox->height;
    uint8_t monster_index;

    switch(entity->direction)
    {
        case DIR_UP:
            entity->world_y -= PLAYER_SPRITES_HEIGHT;
            xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X);
            xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y - PLAYER_SPRITES_HEIGHT);
            hitbox->x = 4;
            hitbox->y = 1;
            hitbox->width = 9;
            hitbox->height = 15;

            break;
        case DIR_DOWN:
            entity->world_y += PLAYER_SPRITES_HEIGHT;
            xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X);
            xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y + PLAYER_SPRITES_HEIGHT);
            hitbox->x = 4;
            hitbox->y = 0;
            hitbox->width = 9;
            hitbox->height = 15;
            
            break;
        case DIR_LEFT:
            entity->world_x -= PLAYER_SPRITES_WIDTH;
            xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X - PLAYER_SPRITES_WIDTH);
            xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y);
            hitbox->x = 1;
            hitbox->y = 7;
            hitbox->width = 15;
            hitbox->height = 9;
            break;
        case DIR_RIGHT:
            entity->world_x += PLAYER_SPRITES_WIDTH;
            xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, x_pos_px, PLAYER_SCREEN_X + PLAYER_SPRITES_WIDTH);
            xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, y_pos_px, PLAYER_SCREEN_Y);
            hitbox->x = 0;
            hitbox->y = 7;
            hitbox->width = 15;
            hitbox->height = 9;
        break;
    }

    entity->collision_on = false;
    monster_index = collision_check_monsters(entity, monsters, 0, 0);
    if (entity->collision_on)
    {
        player_attack_monster(player, monsters, monster_index);
        entity->collision_on = false;
    }
    entity->world_x = curent_world_x;
    entity->world_y = current_world_y;
    hitbox->x = player_hitbox_x;
    hitbox->y = player_hitbox_y;
    hitbox->width = player_hitbox_width;
    hitbox->height = player_hitbox_height;
    if (entity->action_lock_counter++ > 24){
        entity->state = ENTITY_IDLE;
        entity->action_lock_counter = 0;
        entity->attacking = false;
    }
}

static void player_interact_facing(player_t *player, object_t *objects, npc_t *npcs)
{
    entity_t probe;
    int16_t dx = 0;
    int16_t dy = 0;
    uint8_t index;

    probe = player->entity;
    switch (probe.direction)
    {
        case DIR_UP: dy = -1; break;
        case DIR_DOWN: dy = 1; break;
        case DIR_LEFT: dx = -1; break;
        case DIR_RIGHT: dx = 1; break;
        default: return;
    }

    /* Probe ahead without moving the player or changing movement collision. */
    probe.collision_on = false;
    collision_check_tiles(&probe, dx, dy);
    if (probe.collision_on)
        return;
    collision_check_object(&probe, objects, dx, dy);
    if (probe.collision_on)
        return;
    index = collision_check_npcs(&probe, npcs, dx, dy);
    player_interact_npc(player, npcs, index);
}

static void player_interact_npc(player_t *player, npc_t *npcs, uint8_t index)
{
    if (index != 255)
    {
        npc_t *npc = &npcs[index];
        if (input_state.interact_pressed)
        {
            npc_speak(npc, player);
            input_state.interact_pressed = false; // Consume a successful interaction once.
            game_state_set(GAME_STATE_DIALOGUE);

        }
    }
}

static void player_attack_monster(player_t *player, entity_t *monsters, uint8_t index)
{
    entity_t *entity = &player->entity;

    entity_t *monster = &monsters[index];

    if (index != 255)
    {
        entity_damage(monster, entity->attack);
    }
  
}

static void player_interact_monster(player_t *player, entity_t *monsters, uint8_t index)
{
    entity_t *entity = &player->entity;

    if (index != 255)
    {
        player_damage(player, monsters[index].attack);
    }
  
}



void player_damage(player_t *player, uint8_t attack)
{
    entity_t *entity = &player->entity;
    if (!entity->invincible && attack > 0 && entity->life > 0)
    {
        entity->life = attack >= entity->life ? 0 : entity->life - attack;
        entity->invincible = true;
        entity->invincible_counter = 0;
    }
}



static void player_animation_update(player_t *player)
{
    // Update player animation based on state and direction
    entity_t *entity = &player->entity;
    int sprite_index = entity->direction * 4;
    int weapon_sprite_index = PLAYER_EMPTY_FRAME0;

    entity->animation_timer++;
    switch (entity->state)
    {
        case ENTITY_IDLE:
        case ENTITY_DYING:
        case ENTITY_WALKING:
            if (entity->animation_timer >= 12)
            {
                entity->animation_timer = 0;
                entity->animation_frame = (entity->animation_frame + 1) % 4;
            }
        case ENTITY_ATTACKING:
            if (entity->animation_timer >= 6)
            {
                entity->animation_timer = 0;
                entity->animation_frame = (entity->animation_frame + 1) % 4;
            } 
    }
    if (entity->invincible && entity->animation_frame % 2 == 0)
    {
        sprite_index = PLAYER_EMPTY_FRAME0;
    } else 
    {
        switch (entity->state) {
            case ENTITY_IDLE:
                sprite_index = PLAYER_WALK_DIRTECTION_ANIMATION_LOOKUP[entity->direction][0];
                weapon_sprite_index = PLAYER_EMPTY_FRAME0;
                break;
            case ENTITY_WALKING:
                sprite_index = PLAYER_WALK_DIRTECTION_ANIMATION_LOOKUP[entity->direction][entity->animation_frame];
                weapon_sprite_index = PLAYER_EMPTY_FRAME0;

                break;
            case ENTITY_ATTACKING:
                sprite_index = PLAYER_ATTACK_DIRTECTION_LOOKUP[entity->direction];
                weapon_sprite_index = SWORD_DIRTECTION_ANIMATION_LOOKUP[entity->direction][entity->animation_frame];
                break;
            case ENTITY_DYING:
                // Handle dying animation
                break;
        }
    }
    entity->xram_sprite_ptr =  XRAM_PLAYER_IMAGES + sprite_index * BYTES_PER_SPRITE;
    entity->xram_weapon_sprite_ptr = XRAM_PLAYER_IMAGES + weapon_sprite_index * BYTES_PER_SPRITE;

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
    xram0_struct_set(XRAM_SPRITE_CONFIG(PLAYER_WEAPON_SLOT), vga_mode5_sprite_t, xram_sprite_ptr, entity->xram_weapon_sprite_ptr);

}
