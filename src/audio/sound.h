#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
#include <stdbool.h>

#include "../xram_layout.h"


// Call init with a valid xram address to turn on RIA PSG sound.
// Requires 64 bytes of xram.
void sound_init(uint16_t xram_address);

/* Call once per 60 Hz game tick to advance effects and recycle channels. */
void sound_update(void);

/* Generated eight-byte PSG frames in CPU memory; keep data alive until done.
 * Returns the channel's XRAM address, or 0xFFFF if all channels are busy. */
uint16_t sound_play_effect(const uint8_t *frames, uint16_t frame_count);
bool sound_play_door_open(void);


/* freq is Hz * 3; duration/release are 60 Hz ticks; pan is -63..63.
 * vol_attack, vol_decay and wave_release are packed PSG register bytes. */
uint16_t sound_play(uint16_t freq,
                    uint8_t duration,
                    uint8_t release,
                    uint8_t duty,
                    uint8_t vol_attack,
                    uint8_t vol_decay,
                    uint8_t wave_release,
                    int8_t pan);


#endif // SOUND_H
