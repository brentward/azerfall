#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include <stdbool.h>

#include "../game/entity.h"

struct player_t;

typedef enum {
    OBJECT_CHEST,
    OBJECT_DOOR,
    OBJECT_KEY
} object_type_t;

typedef enum {
    CHEST_CLOSED,
    CHEST_OPEN,
    DOOR_CLOSED,
    DOOR_OPENED,
    KEY_UNCOLLECTED,
    KEY_COLLECTED
} object_state_t;


typedef struct {
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