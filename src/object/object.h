#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include <stdbool.h>

#include "../entity/entity.h"

struct player_t;

typedef enum {
    OBJECT_CHEST,
    OBJECT_DOOR,
    OBJECT_KEY,
    OBJECT_HEART
} object_type_t;

typedef enum {
    CHEST_CLOSED = 0,
    CHEST_OPEN = 1,
    DOOR_CLOSED = 2,
    DOOR_OPENED = 3,
    KEY_UNCOLLECTED = 4,
    KEY_COLLECTED = 4,
    HEART_FULL = 5,
    HEART_THREE_QUARTER = 6,
    HEART_HALF = 7,
    HEART_QUARTER = 8,
    HEART_EMPTY = 9
} object_state_t;


typedef struct object_t {
    object_type_t type;

    int16_t world_x;
    int16_t world_y;
    int16_t screen_x;
    int16_t screen_y;

    hitbox_t hitbox;

    object_state_t state;
    uint16_t xram_sprite_ptr;

    bool collision;
} object_t;


void init_chest(object_t *chest, struct player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void init_door(object_t *door, struct player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void init_key(object_t *key, struct player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void object_sprite_init(void);
void object_prepare_draw(object_t *obj, struct player_t *player);
void object_draw(object_t *obj, uint8_t config_slot);

#endif // OBJECT_H