#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#include "entity.h"
#include "game.h"
#include "../object/object.h"

#define PLAYER_SPRITE_SLOT 0U
#define PLAYER_PALETTE 0xFFFF
#define PLAYER_SCREEN_X SCREEN_WIDTH / 2 - HALF_TILE_SIZE
#define PLAYER_SCREEN_Y SCREEN_HEIGHT / 2 - HALF_TILE_SIZE


typedef enum {
    PLAYER_IDLE,
    PLAYER_WALKING,
    PLAYER_ATTACKING,
    PLAYER_DYING
} PlayerState;


typedef struct Player {
    Entity entity;
    PlayerState state;

    int16_t screen_org_x;
    int16_t screen_org_y;

    uint8_t key_count;
    uint8_t treasure_count;

} Player;

void player_init(Player *player);
void player_graphics_init(void);
void player_update(Player *player, GameObject objects[OBJECT_COUNT]);
void player_draw(Player *player);

#endif // PLAYER_H