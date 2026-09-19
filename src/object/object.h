#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include <stdbool.h>

#include "../game/entity.h"

struct Player;

typedef enum {
    OBJECT_CHEST,
    OBJECT_DOOR,
    OBJECT_KEY
} ObjectType;

typedef enum {
    CHEST_CLOSED,
    CHEST_OPEN,
    DOOR_CLOSED,
    DOOR_OPENED,
    KEY_UNCOLLECTED,
    KEY_COLLECTED
} ObjectState;


typedef struct {
    ObjectType type;

    int16_t world_x;
    int16_t world_y;
    int16_t screen_x;
    int16_t screen_y;

    HitBox hitbox;

    ObjectState state;
    uint16_t xram_sprite_ptr;

    bool collision;
} GameObject;


void init_chest(GameObject *chest, struct Player *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void init_door(GameObject *door, struct Player *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void init_key(GameObject *key, struct Player *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void object_sprite_init(void);
void object_prepare_draw(GameObject *obj, struct Player *player);
void object_draw(GameObject *obj, uint8_t config_slot);

#endif // OBJECT_H