#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdbool.h>

#define PLAYER_SPRITES (BACKGROUND_TILES + GRASS_PATH_TILES_TOTAL_BYTES)
#define PLAYER_SPRITE_CONFIG 0xFF40U // Keep clear of keyboard data at 0xFF10-0xFF2F.
#define PLAYER_PALETTE 0xFFFF

typedef enum {
    PLAYER_IDLE,
    PLAYER_WALKING,
    PLAYER_ATTACKING,
    PLAYER_DYING
} PlayerState;

typedef enum {
    DIR_DOWN,
    DIR_DOWN_LEFT,
    DIR_LEFT,
    DIR_UP_LEFT,
    DIR_UP,
    DIR_UP_RIGHT,
    DIR_RIGHT,
    DIR_DOWN_RIGHT
} Direction;

typedef struct {
    int16_t x;
    int16_t y;

    int8_t speed;

    Direction direction;
    PlayerState state;

    uint8_t animation_frame;
    uint8_t animation_timer;

} Player;

extern Player player;

void player_init(void);
void player_update(void);
void player_animation_update(void);
void player_draw(void);

#endif // PLAYER_H