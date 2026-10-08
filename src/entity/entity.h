#ifndef ENTITY_H
#define ENTITY_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define DIR_DIVISOR ((RAND_MAX + 1U) / 4U)
#define DIR_UP_DIVIDE ((DIR_DIVISOR * 1U) - 1U)
#define DIR_DOWN_DIVIDE ((DIR_DIVISOR * 2U) - 1U)
#define DIR_LEFT_DIVIDE ((DIR_DIVISOR * 3U) - 1U)
#define DIR_RIGHT_DIVIDE ((DIR_DIVISOR * 4U) - 1U)


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
    DIR_NONE = 1,
    DIR_UP = 0,
    DIR_DOWN = 1,
    DIR_LEFT = 2,
    DIR_RIGHT = 3,
    DIR_ANY = 4
} direction_t;

typedef enum {
    ENTITY_IDLE,
    ENTITY_WALKING,
    ENTITY_ATTACKING,
    ENTITY_DYING
} entity_state_t;

typedef enum {
    ENTITY_PLAYER = 0,
    ENTITY_NPC_OLDMAN = 1,
    ENTITY_NPC_MERCHANT = 2,
    ENTITY_MONSTER_GREENSLIME = 3
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
    uint8_t attack;
    uint8_t speed;
    direction_t direction;
    bool collision_on;
    bool invincible;
    uint8_t invincible_couunter;

    uint8_t animation_frame;
    uint8_t animation_timer;
    uint8_t action_lock_counter;
    uint16_t xram_sprite_ptr;
    
    hitbox_t hitbox;

} entity_t;

void entity_update(entity_t *entity, struct player_t *player, struct object_t *objects, struct npc_t *npcs, entity_t *monsters);

void entity_prepare_draw(entity_t *entity, struct player_t *player);
void entity_draw(entity_t *entity, uint8_t config_slot);

#endif // ENTITY_H
