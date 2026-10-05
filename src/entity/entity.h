#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <stdbool.h>

struct player_t;
struct object_t;
struct npc_t;

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t width;
    uint8_t height;
} hitbox_t;

typedef enum {
    DIR_NONE = 0,
    DIR_DOWN = 0,
    DIR_DOWN_LEFT = 1,
    DIR_LEFT = 2,
    DIR_UP_LEFT = 3,
    DIR_UP = 4,
    DIR_UP_RIGHT = 5,
    DIR_RIGHT = 6,
    DIR_DOWN_RIGHT = 7,
    DIR_ANY = 8
} direction_t;

typedef enum {
    ENTITY_IDLE,
    ENTITY_WALKING,
    ENTITY_ATTACKING,
    ENTITY_DYING
} entity_state_t;

typedef enum {
    ENTITY_NPC_OLDMAN = 0,
    ENTITY_PLAYER = 1,
    ENTITY_NPC_MERCHANT = 8
} entity_type_t;


typedef struct {
    entity_type_t type;
    entity_state_t state;
    int16_t world_x;
    int16_t world_y;
    int16_t screen_x;
    int16_t screen_y;

    uint8_t max_life;
    uint8_t life;
    uint8_t speed;
    direction_t direction;
    bool collision_on;

    uint8_t animation_frame;
    uint8_t animation_timer;
    uint8_t action_lock_counter;
    uint16_t xram_sprite_ptr;
    
    hitbox_t hitbox;

} entity_t;

void entity_update(entity_t *entity, struct player_t *player, struct object_t *objects, struct npc_t *npcs);

void entity_prepare_draw(entity_t *entity, struct player_t *player);
void entity_draw(entity_t *entity, uint8_t config_slot);

#endif // ENTITY_H
