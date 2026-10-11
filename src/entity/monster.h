#ifndef MONSTER_H
#define MONSTER_H

#include "entity.h"
#include "../../generated/monster_sprites.h"


#define GREENSLIME_FRAME0 MONSTER_DIR0_FRAME0
#define GREENSLIME_FRAME1 MONSTER_DIR0_FRAME1
#define GREENSLIME_FRAME2 MONSTER_DIR0_FRAME0
#define GREENSLIME_FRAME3 MONSTER_DIR0_FRAME2

#define MONSTER_DYING_FRAME0 MONSTER_DIR0_FRAME3
#define MONSTER_DYING_FRAME1 MONSTER_DIR1_FRAME0
#define MONSTER_DYING_FRAME2 MONSTER_DIR1_FRAME1
#define MONSTER_DYING_FRAME3 MONSTER_DIR1_FRAME2
#define MONSTER_EMPTY_FRAME MONSTER_DIR1_FRAME3

#define MONSTER_DYING_ANIMATION_FRAMES 4

static const int MONSTER_DYING_ANIMATION_LOOKUP[MONSTER_DYING_ANIMATION_FRAMES] = { MONSTER_DYING_FRAME0,  MONSTER_DYING_FRAME1,  MONSTER_DYING_FRAME2,  MONSTER_DYING_FRAME3 };

struct player_t;

void monster_sprite_init(void);
void monster_greenslime_init(entity_t *greenslime, struct player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void monster_greenslime_update(entity_t *greenslime);

#endif // MONSTER_H
