#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdbool.h>
#include "player.h"
#include "entity.h"

#define PLAYER_SPRITES (BACKGROUND_TILES + WORLD01_TILES_MAP_TOTAL_BYTES)
#define PLAYER_SPRITE_CONFIG 0xFF40U // Keep clear of keyboard data at 0xFF10-0xFF2F.
#define PLAYER_PALETTE 0xFFFF
#define PLAYER_SCREEN_X SCREEN_WIDTH / 2 - HALF_TILE_SIZE
#define PLAYER_SCREEN_Y SCREEN_HEIGHT / 2 - HALF_TILE_SIZE


typedef enum {
    PLAYER_IDLE,
    PLAYER_WALKING,
    PLAYER_ATTACKING,
    PLAYER_DYING
} PlayerState;


typedef struct {
    Entity entity;
    PlayerState state;

    uint8_t animation_frame;
    uint8_t animation_timer;

} Player;

extern Player player;

void player_init(void);
void player_update(void);
void player_animation_update(void);

#endif // PLAYER_H