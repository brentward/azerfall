#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t width;
    uint8_t height;
} HitBox;

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
    int16_t world_x;
    int16_t world_y;

    uint8_t speed;
    Direction direction;

    HitBox hitbox;
} Entity;

#endif // ENTITY_H