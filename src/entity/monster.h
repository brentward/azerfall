#ifndef MONSTER_H
#define MONSTER_H

#include "entity.h"


#define GREENSLIME_FRAME0 MONSTER_DIR0_FRAME0
#define GREENSLIME_FRAME1 MONSTER_DIR0_FRAME1
#define GREENSLIME_FRAME2 MONSTER_DIR0_FRAME0
#define GREENSLIME_FRAME3 MONSTER_DIR0_FRAME2

struct player_t;

void monster_sprite_init(void);
void monster_greenslime_init(entity_t *greenslime, struct player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void monster_greenslime_update(entity_t *greenslime);

#endif // MONSTER_H
