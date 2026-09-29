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
} player_state_t;


typedef struct player_t {
    entity_t entity;
    player_state_t state;

    int16_t screen_org_x;
    int16_t screen_org_y;

    uint8_t key_count;
} player_t;

void player_init(player_t *player);
void player_graphics_init(void);
void player_update(player_t *player, object_t objects[OBJECT_COUNT]);
void player_draw(player_t *player);

#endif // PLAYER_H