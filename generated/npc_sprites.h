#ifndef NPC_SPRITES_H
#define NPC_SPRITES_H

#include <stdint.h>

#define NPC_SPRITES_WIDTH 16
#define NPC_SPRITES_HEIGHT 16
#define NPC_SPRITES_BPP 4
#define NPC_SPRITES_BYTES_PER_SPRITE 128
#define NPC_SPRITES_COUNT 10
#define NPC_SPRITES_TOTAL_BYTES 1280

#define NPC_IMAGE0 0
#define NPC_IMAGE1 1
#define NPC_IMAGE2 2
#define NPC_IMAGE3 3
#define NPC_IMAGE4 4
#define NPC_IMAGE5 5
#define NPC_IMAGE6 6
#define NPC_IMAGE7 7
#define NPC_IMAGE8 8
#define NPC_IMAGE9 9

extern const uint8_t npc_sprites[NPC_SPRITES_TOTAL_BYTES];

#define NPC_SPRITES_PALETTE_COUNT 16
#define NPC_SPRITES_PALETTE_BYTES (NPC_SPRITES_PALETTE_COUNT * 2)
extern const uint16_t npc_sprites_palette[NPC_SPRITES_PALETTE_COUNT];

#endif
