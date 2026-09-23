#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>
#include <stdint.h>


#define DOOR_FRAMES 48   /* Main burst at 100 ms, fading out by 800 ms */
#define DOOR_MOD_LEVEL 0x04 /* Lower = rougher */
#define DOOR_FNUM 580
#define DOOR_BLOCK 2    /* Decrease by 1 to lower one octave */

#define PICKUP_LOW_FNUM 326   /* Approximately B6 (1978 Hz), block 6 */
#define PICKUP_HIGH_FNUM 435  /* Approximately E7 (2640 Hz), block 6 */
#define PICKUP_BLOCK 6       /* Set to 5 for the original octave */
#define PICKUP_MOD_LEVEL 0x12 /* Lower = brighter; previous setting was 0x16 */
#define PICKUP_LEVEL 6
#define PICKUP_FIRST_FRAMES 6
#define PICKUP_HOLD_FRAMES 14
#define PICKUP_FADE_FRAMES 36


typedef enum {
    SFX_NONE,
    SFX_DOOR,
    SFX_PICKUP
} SfxId;



void sound_init(void);
void sound_play(SfxId sound);
void sound_update(void);
bool is_sfx_register(uint8_t reg);


#endif
