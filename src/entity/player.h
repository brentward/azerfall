#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#include "../xram.h"
#include "npc.h"
#include "../game/game.h"
#include "../object/object.h"

#define PLAYER_SPRITE_SLOT 0U
#define PLAYER_PALETTE 0xFFFF
#define PLAYER_SCREEN_X SCREEN_WIDTH / 2 - HALF_TILE_SIZE
#define PLAYER_SCREEN_Y SCREEN_HEIGHT / 2 - HALF_TILE_SIZE

typedef struct player_t {
    entity_t entity;
    int16_t screen_org_x;
    int16_t screen_org_y;

    uint8_t key_count;
} player_t;

void player_init(player_t *player);
void player_graphics_init(void);
void player_update(player_t *player, object_t objects[OBJECT_COUNT], npc_t npcs[NPC_COUNT]);
void player_draw(player_t *player);

#endif // PLAYER_H