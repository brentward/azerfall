#ifndef NPC_H
#define NPC_H

#include "entity.h"
#include "../../generated/npc_sprites.h"

/* Sheet cells: down, up, left, right, then merchant; two frames each. */
#define OLDMAN_DIR0_FRAME0 NPC_IMAGE0
#define OLDMAN_DIR0_FRAME1 NPC_IMAGE1
#define OLDMAN_DIR1_FRAME0 NPC_IMAGE2
#define OLDMAN_DIR1_FRAME1 NPC_IMAGE3
#define OLDMAN_DIR2_FRAME0 NPC_IMAGE4
#define OLDMAN_DIR2_FRAME1 NPC_IMAGE5
#define OLDMAN_DIR3_FRAME0 NPC_IMAGE6
#define OLDMAN_DIR3_FRAME1 NPC_IMAGE7
#define MERCHANT_DIR0_FRAME0 NPC_IMAGE8
#define MERCHANT_DIR0_FRAME1 NPC_IMAGE9

struct player_t;

typedef struct npc_t {
    entity_t entity;
    uint8_t dialogue_index;
    char *dialogue_lines[4];
} npc_t;

void npc_sprite_init(void);
void npc_oldman_init(entity_t *oldman, struct player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void npc_merchant_init(entity_t *merchant, struct player_t *player, uint8_t config_slot, int16_t world_x, int16_t world_y);
void npc_oldman_update(entity_t *oldman);
void npc_oldman_set_dialogue(npc_t *oldman, char *line1, char *line2, char *line3, char *line4);
void npc_speak(npc_t *npc, struct player_t *player);
void npc_merchant_update(entity_t *merchant);

#endif // NPC_H
