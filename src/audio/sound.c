#include "sound.h"

#include <stdint.h>

#include <rp6502.h>

#include "../xram.h"

#define SFX_CHANNEL_COUNT 2
#define SFX_REGISTER_COUNT 26

typedef struct
{
    SfxId sound;
    uint8_t frame;
    uint8_t delay;
} SfxVoice;

static SfxVoice voices[SFX_CHANNEL_COUNT];  /* OPL channels 7 and 8 */

uint8_t sfx_registers [SFX_REGISTER_COUNT] = {
    /* Channel 7 */
0x31, 0x34,  // Operator settings
0x51, 0x54,  // Volume
0x71, 0x74,  // Attack/decay
0x91, 0x94,  // Sustain/release
0xF1, 0xF4,  // Waveforms
0xA7, 0xB7,  // Pitch and note on/off
0xC7,        // Feedback/connection

/* Channel 8 */
0x32, 0x35,
0x52, 0x55,
0x72, 0x75,
0x92, 0x95,
0xF2, 0xF5,
0xA8, 0xB8,
0xC8
};

static const uint8_t door_level[DOOR_FRAMES] = {
    0, 5, 9, 15, 23, 45, 0, 0,
    1, 1, 2, 2, 3, 3, 4, 4,
    5, 5, 6, 6, 7, 8, 8, 9,
    10, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 22, 23, 25, 27,
    29, 32, 34, 38, 42, 48, 57, 63
};

static const uint8_t mod_attenuation[DOOR_FRAMES / 2] = {
    0, 3, 6, 0, 0, 0, 1, 1,
    2, 2, 3, 3, 4, 4, 5, 5,
    6, 6, 7, 8, 9, 10, 11, 12
};



static void opl_write(uint8_t reg, uint8_t value)
{
    RIA.addr1 = XRAM_OPL + reg;
    RIA.rw1 = value;
}

static void door_patch(uint8_t channel)
{
    /* Fast, heavily fed-back modulation over a low carrier aims for
     * continuous friction rather than individually audible pulses.
     * Sustained envelopes let the frame sequence shape the attack/fade.
     * Sine waveforms need no global waveform-enable changes.
     */
    opl_write(0x31 + channel, 0x28); /* Modulator multiplier 8: denser texture */
    opl_write(0x51 + channel, DOOR_MOD_LEVEL);
    opl_write(0x71 + channel, 0xF0);
    opl_write(0x91 + channel, 0x0A);
    opl_write(0xF1 + channel, 0x00);

    opl_write(0x34 + channel, 0x20); /* Carrier multiplier 0.5: low rumble */
    opl_write(0x54 + channel, 0x00); /* Full-strength initial attack */
    opl_write(0x74 + channel, 0xF0);
    opl_write(0x94 + channel, 0x0A);
    opl_write(0xF4 + channel, 0x00);
    opl_write(0xC7 + channel, 0x0E); /* Maximum feedback, FM */
}

static void pickup_patch(uint8_t channel)
{
    /* 2:1 FM emphasizes odd harmonics for a square-like coin tone.
     * Sustain both operators; playback controls the fade so the timbre
     * stays bright instead of turning into a soft bell during decay.
     */
    opl_write(0x31 + channel, 0x22);  /* Sustained envelope, multiplier 2 */
    opl_write(0x51 + channel, PICKUP_MOD_LEVEL); /* Stronger upper harmonics */
    opl_write(0x71 + channel, 0xF0);  /* Instant attack, no automatic decay */
    opl_write(0x91 + channel, 0x0A);  /* Full sustain, quick final release */
    opl_write(0xF1 + channel, 0x00);  /* Sine */

    opl_write(0x34 + channel, 0x21);  /* Sustained envelope, multiplier 1 */
    opl_write(0x54 + channel, PICKUP_LEVEL);
    opl_write(0x74 + channel, 0xF0);  /* Instant attack, no automatic decay */
    opl_write(0x94 + channel, 0x0A);  /* Full sustain, quick final release */
    opl_write(0xF4 + channel, 0x00);  /* Sine */
    opl_write(0xC7 + channel, 0x00);  /* No feedback, FM connection */
}

// static void key_pickup_patch(uint8_t channel)
// {
//     /* Sine-wave FM: a 3:1 ratio gives a bright, bell-like attack. */
//     opl_write(0x31 + channel, 0x03);  /* Modulator: percussive envelope, multiplier 3 */
//     opl_write(0x51 + channel, 0x18);  /* Modulation depth; lower = more metallic */
//     opl_write(0x71 + channel, 0xFA);  /* Instant attack, fast decay of brightness */
//     opl_write(0x91 + channel, 0x88);  /* Sustain attenuation 8, release 8 */
//     opl_write(0xF1 + channel, 0x00);  /* Sine */

//     opl_write(0x34 + channel, 0x01);  /* Carrier: percussive envelope, multiplier 1 */
//     opl_write(0x54 + channel, 0x06);  /* Slightly reduced output level */
//     opl_write(0x74 + channel, 0xF7);  /* Instant attack, gentler decay */
//     opl_write(0x94 + channel, 0x78);  /* Sustain attenuation 7, release 8 */
//     opl_write(0xF4 + channel, 0x00);  /* Sine */
//     opl_write(0xC7 + channel, 0x00);  /* No feedback, FM connection */
// }

static void opl_set_pitch(uint16_t fnum, uint8_t block, uint8_t key_on, uint8_t channel)
{
    opl_write(0xA7 + channel, (uint8_t)fnum);
    opl_write(0xB7 + channel,
              (key_on ? 0x20 : 0x00) |
              (block << 2) |
              ((fnum >> 8) & 0x03));
}

void sound_play(SfxId sound)
{
    uint8_t i;

    for (i = 0; i < SFX_CHANNEL_COUNT; i++)
    {
        if (voices[i].sound == SFX_NONE)
        {
            break;
        }
    }
    
    /* Both slots are occupied: drop the request without touching either. */
    if (i == SFX_CHANNEL_COUNT)
        return;

    switch (sound)
    {
        case SFX_DOOR:
            voices[i].sound = SFX_DOOR;
            voices[i].frame = 0;
            voices[i].delay = 0;
            break;

        case SFX_PICKUP:
            voices[i].sound = SFX_PICKUP;
            voices[i].frame = 0;
            voices[i].delay = 0;
            break;

        default:
            voices[i].sound = SFX_NONE;
            break;
    }
}

void sound_update(void)
{
    uint8_t i;

    for (i = 0; i < SFX_CHANNEL_COUNT; i++)
    {
        if (voices[i].sound == SFX_NONE)
            continue;

        if (voices[i].delay > 0)
        {
            --voices[i].delay;
            continue;
        }

        switch (voices[i].sound)
        {
            case SFX_DOOR:
                if (voices[i].frame == 0)
                {
                    opl_set_pitch(DOOR_FNUM, DOOR_BLOCK, 0, i);
                }
                else if (voices[i].frame <= DOOR_FRAMES)
                {
                    if (voices[i].frame == 1)
                    {
                        door_patch(i);
                        opl_set_pitch(DOOR_FNUM, DOOR_BLOCK, 1, i);
                    }
                    /* Envelope index 0 starts on the same update as key-on. */
                    opl_write(0x54 + i, door_level[voices[i].frame - 1]);
                    opl_write(0x51 + i,
                              DOOR_MOD_LEVEL + mod_attenuation[(voices[i].frame - 1) >> 1]);
                }
                else
                {
                    opl_set_pitch(DOOR_FNUM, DOOR_BLOCK, 0, i);
                    voices[i].sound = SFX_NONE;
                    continue;
                }
                ++voices[i].frame;
                /* One update per frame: no extra skipped updates. */
                break;
            case SFX_PICKUP:
                switch (voices[i].frame)
                {
                    case 0:
                        opl_set_pitch(PICKUP_LOW_FNUM, PICKUP_BLOCK, 0, i);
                        /* Next update is one frame later; skip no updates. */
                        voices[i].delay = 0;
                        break;

                    case 1:
                        pickup_patch(i);
                        opl_set_pitch(PICKUP_LOW_FNUM, PICKUP_BLOCK, 1, i);
                        voices[i].delay = PICKUP_FIRST_FRAMES - 1;
                        break;

                    case 2:
                        opl_set_pitch(PICKUP_HIGH_FNUM, PICKUP_BLOCK, 1, i);
                        voices[i].delay = PICKUP_HOLD_FRAMES - 1;
                        break;

                    default:
                        /* Steps 3..38 apply fade levels 1..36, one per frame.
                         * Keep key-on and the modulator unchanged during fade. */
                        if (voices[i].frame < 3 + PICKUP_FADE_FRAMES)
                        {
                            opl_write(0x54 + i, PICKUP_LEVEL + voices[i].frame - 2);
                            break;
                        }
                        opl_set_pitch(PICKUP_HIGH_FNUM, PICKUP_BLOCK, 0, i);
                        voices[i].sound = SFX_NONE;
                        continue;
                }
                ++voices[i].frame;
                break;

            default:
                voices[i].sound = SFX_NONE;
                break;
        }
    }
}

bool is_sfx_register(uint8_t reg)
{
    uint8_t i;

    for (i = 0; i < SFX_REGISTER_COUNT; i++)
    {
        if (reg == sfx_registers[i])
            return true;
    }
    return false;
}

void sound_init(void)
{
    uint8_t i;
    /* Shared chip settings belong to the game, not the music stream. */
    opl_write(0x01, 0x20); /* Enable the waveforms selected by each patch. */
    opl_write(0x08, 0x00); /* Normal note-select mode; CSM disabled. */
    opl_write(0xBD, 0x00); /* Nine melodic channels; rhythm mode disabled. */
    opl_write(0xB7, 0x00);
    opl_write(0xB8, 0x00);
    
    for (i = 0; i < SFX_CHANNEL_COUNT; i++)
    {
        voices[i].sound = SFX_NONE;
        voices[i].frame = 0;
        voices[i].delay = 0;
    }
}
