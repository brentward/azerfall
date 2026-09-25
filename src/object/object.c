#include "object.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../xram.h"
#include "../graphics/graphics.h"
#include "../game/player.h"

static ScreenPosition get_screen_position(GameObject *obj, Player *player);

void init_chest(GameObject *chest, Player *player, uint8_t config_slot, int16_t world_x, int16_t world_y)
{
    Entity *entity = &player->entity;
    ScreenPosition screen_position;


    HitBox *hitbox;
    ObjectType *type;
    ObjectState *state;
    memset(chest, 0, sizeof *chest);
    hitbox = &chest->hitbox;
    type = &chest->type;
    state = &chest->state;
    hitbox->x = 0;
    hitbox->y = 0;
    hitbox->width = 16;
    hitbox->height = 16;
    chest->type = OBJECT_CHEST;
    chest->state = CHEST_CLOSED;
    chest->world_x = world_x;
    chest->world_y = world_y;
    chest->collision = false;
    chest->xram_sprite_ptr = XRAM_OBJECT_IMAGES + chest->state * BYTES_PER_SPRITE;
    screen_position = get_screen_position(chest, player);
    chest->screen_x = screen_position.screen_x;
    chest->screen_y = screen_position.screen_y;

    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, x_pos_px, chest->screen_x);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, y_pos_px, chest->screen_y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, xram_sprite_ptr, XRAM_OBJECT_IMAGES + chest->state * BYTES_PER_SPRITE);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, palette_ptr, XRAM_WORLD_PALETTE);
}

void init_door(GameObject *door, Player *player, uint8_t config_slot, int16_t world_x, int16_t world_y)
{
    Entity *entity = &player->entity;
    ScreenPosition screen_position;


    HitBox *hitbox;
    ObjectType *type;
    ObjectState *state;
    memset(door, 0, sizeof *door);
    hitbox = &door->hitbox;
    type = &door->type;
    state = &door->state;
    hitbox->x = 0;
    hitbox->y = 0;
    hitbox->width = 16;
    hitbox->height = 16;
    door->type = OBJECT_DOOR;
    door->state = DOOR_CLOSED;
    door->world_x = world_x;
    door->world_y = world_y;
    door->collision = true;
    door->xram_sprite_ptr = XRAM_OBJECT_IMAGES + door->state * BYTES_PER_SPRITE;
    screen_position = get_screen_position(door, player);
    door->screen_x = screen_position.screen_x;
    door->screen_y = screen_position.screen_y;

    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, x_pos_px, door->screen_x);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, y_pos_px, door->screen_y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, xram_sprite_ptr, door->xram_sprite_ptr);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, palette_ptr, XRAM_WORLD_PALETTE);
}

void init_key(GameObject *key, Player *player, uint8_t config_slot, int16_t world_x, int16_t world_y)
{
    Entity *entity = &player->entity;
    ScreenPosition screen_position;

    HitBox *hitbox;
    ObjectType *type;
    ObjectState *state;
    memset(key, 0, sizeof *key);
    hitbox = &key->hitbox;
    type = &key->type;
    state = &key->state;
    hitbox->x = 0;
    hitbox->y = 0;
    hitbox->width = 16;
    hitbox->height = 16;
    key->type = OBJECT_KEY;
    key->state = KEY_UNCOLLECTED;
    key->world_x = world_x;
    key->world_y = world_y;
    key->collision = false;
    key->xram_sprite_ptr = XRAM_OBJECT_IMAGES + key->state * BYTES_PER_SPRITE;
    screen_position = get_screen_position(key, player);
    key->screen_x = screen_position.screen_x;
    key->screen_y = screen_position.screen_y;

    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, x_pos_px, key->screen_x);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, y_pos_px, key->screen_y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, xram_sprite_ptr, key->xram_sprite_ptr);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, palette_ptr, XRAM_WORLD_PALETTE);

}

static ScreenPosition get_screen_position(GameObject *obj, Player *player)
{
    ScreenPosition screen_position;
    Entity *entity = &player->entity;

    screen_position.screen_x = obj->world_x - entity->world_x + PLAYER_SCREEN_X;
    screen_position.screen_y = obj->world_y - entity->world_y + PLAYER_SCREEN_Y;

    return screen_position;
}

void object_sprite_init(void)
{
    int i;

    RIA.addr0 = XRAM_OBJECT_IMAGES;
    RIA.step0 = 1;
    for (i = 0; i < OBJECT_SPRITES_TOTAL_BYTES; i++)
    {
        RIA.rw0 = object_sprites[i];
    }
}

void object_prepare_draw(GameObject *obj, Player *player)
{
    ScreenPosition screen_position;
    screen_position = get_screen_position(obj, player);

    obj->screen_x = screen_position.screen_x;
    obj->screen_y = screen_position.screen_y;
    obj->xram_sprite_ptr = XRAM_OBJECT_IMAGES + obj->state * BYTES_PER_SPRITE;
}


void object_draw(GameObject *obj, uint8_t config_slot)
{    
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, x_pos_px, obj->screen_x);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, y_pos_px, obj->screen_y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, xram_sprite_ptr, obj->xram_sprite_ptr);
}