#ifndef WEAPON_H
#define WEAPON_H

#include <stdint.h>

#define WEAPON_SPRITE_CONFIG_SLOT PAUSE_SPRITE_SLOT_START
#define SHIELD_SPRITE_CONFIG_SLOT PAUSE_SPRITE_SLOT_START + 1

struct player_t;

typedef enum {
    WEAPON_NONE,
    SWORD_NORMAL
} weapon_type_t;

typedef enum {
    SHIELD_NONE,
    SHIELD_WOOD
} shield_type_t;

typedef struct {
    weapon_type_t weapon_type;
    uint8_t attack_value;
} weapon_t;

typedef struct {
    shield_type_t shield_type;
    uint8_t defense_value;
} shield_t;

void weapon_sprite_init(void);
void weapon_normal_sword_init(struct player_t *player);

#endif // WEAPON_H