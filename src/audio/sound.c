#include "sound.h"

#include <stdint.h>
#include <stdio.h>

#include <rp6502.h>

#include "../xram.h"

#define SFX_CHANNEL_COUNT 2

typedef struct
{
    sfx_id_t sound;
    uint8_t frame;
    uint8_t delay;
} sfx_voice_t;

static sfx_voice_t voices[SFX_CHANNEL_COUNT];  /* OPL channels 7 and 8 */

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

/* A bright initial snap drops into the reference's rising noisy swish.
 * Omit the WAV's leading/trailing silence for responsive play. */
static const uint16_t sword_fnum[SWORD_FRAMES] = {
    600, 380, 250, 270, 300, 360, 440, 540, 650, 700
};
static const uint8_t sword_level[SWORD_FRAMES] = {
    /* Lower OPL attenuation = louder: full attack, settling over 50 ms. */
    0, 5, 10, 20, 16, 14, 12, 14, 17, 23
};

/* receivedamage.wav: rough attack, steep downward sweep, short noisy tail.
 * This is a synthesized approximation, not sample playback. */
static const uint16_t damage_fnum[DAMAGE_FRAMES] = {
    900, 640, 780, 350, 490, 140, 680, 260, 180
};
static const uint8_t damage_level[DAMAGE_FRAMES] = {
    3, 12, 2, 15, 4, 10, 18, 28, 42
};

/* F6, A#5, F6, A#6, rounded to the nearest OPL frequency number. */
static const uint16_t level_up_fnum[4] = { 460, 307, 460, 615 };



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

static void sword_patch(uint8_t channel)
{
    /* Maximum feedback makes the modulator noise-like; strong FM gives a
     * coarse retro swish. Sustain lets the frame envelope shape the burst. */
    opl_write(0x31 + channel, 0x21);
    opl_write(0x51 + channel, SWORD_MOD_LEVEL);
    opl_write(0x71 + channel, 0xF0);
    opl_write(0x91 + channel, 0x0F);
    opl_write(0xF1 + channel, 0x00);

    opl_write(0x34 + channel, 0x21);
    opl_write(0x54 + channel, sword_level[0]);
    opl_write(0x74 + channel, 0xF0);
    opl_write(0x94 + channel, 0x0F);
    opl_write(0xF4 + channel, 0x00);
    opl_write(0xC7 + channel, 0x0E);
}

static void damage_patch(uint8_t channel)
{
    /* Quarter-sine modulation with maximum feedback and a half-frequency
     * modulator breaks up the tone; half-sine carrier adds hard edges.
     * Uneven pitch/level steps above give the short burst a crackling attack. */
    opl_write(0x31 + channel, 0x20);
    opl_write(0x51 + channel, DAMAGE_MOD_LEVEL);
    opl_write(0x71 + channel, 0xF0);
    opl_write(0x91 + channel, 0x0F);
    opl_write(0xF1 + channel, 0x03);

    opl_write(0x34 + channel, 0x21);
    opl_write(0x54 + channel, damage_level[0]);
    opl_write(0x74 + channel, 0xF0);
    opl_write(0x94 + channel, 0x0F);
    opl_write(0xF4 + channel, 0x01);
    opl_write(0xC7 + channel, 0x0E);
}

static void level_up_patch(uint8_t channel)
{
    /* Mostly sine with light 1:1 FM: soft, hollow flute-like tone.
     * Restore both waveforms/feedback after the crunchy damage patch. */
    opl_write(0x31 + channel, 0x21);
    opl_write(0x51 + channel, 0x32);
    opl_write(0x71 + channel, 0xB0);
    opl_write(0x91 + channel, 0x08);
    opl_write(0xF1 + channel, 0x00);
    opl_write(0x34 + channel, 0x21);
    opl_write(0x54 + channel, 0x06);
    opl_write(0x74 + channel, 0xB0);
    opl_write(0x94 + channel, 0x08);
    opl_write(0xF4 + channel, 0x00);
    opl_write(0xC7 + channel, 0x00);
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

void sound_play(sfx_id_t sound)
{
    uint8_t i;
    sfx_voice_t *voice = voices;

    for (i = 0; i < SFX_CHANNEL_COUNT; i++, voice++)
    {
        if (voice->sound == SFX_NONE)
        {
            break;
        }
    }
    
    /* Damage feedback and level-up take priority over the second busy slot.
     * Frame zero keys it off before installing the new patch. */
    if (i == SFX_CHANNEL_COUNT)
    {
        if (sound != SFX_DAMAGE && sound != SFX_LEVEL_UP)
            return;
        voice = &voices[SFX_CHANNEL_COUNT - 1];
    }

    switch (sound)
    {
        case SFX_DOOR:
            voice->sound = SFX_DOOR;
            voice->frame = 0;
            voice->delay = 0;
            break;

        case SFX_PICKUP:
            voice->sound = SFX_PICKUP;
            voice->frame = 0;
            voice->delay = 0;
            break;

        case SFX_LEVEL_UP:
        case SFX_MONSTER_DAMAGE:
        case SFX_DAMAGE:
        case SFX_SWORD:
            voice->sound = sound;
            voice->frame = 0;
            voice->delay = 0;
            break;

        default:
            voice->sound = SFX_NONE;
            break;
    }
}

void sound_update(void)
{
    uint8_t i;
    uint8_t damage_index;
    sfx_voice_t *voice = voices;

    for (i = 0; i < SFX_CHANNEL_COUNT; i++, voice++)
    {
        if (voice->sound == SFX_NONE)
            continue;

        if (voice->delay > 0)
        {
            --voice->delay;
            continue;
        }

        switch (voice->sound)
        {
            case SFX_LEVEL_UP:
                if (voice->frame == 0)
                    opl_set_pitch(level_up_fnum[0], LEVEL_UP_BLOCK, 0, i);
                else if (voice->frame <= 8)
                {
                    if (voice->frame & 1)
                    {
                        if (voice->frame == 1)
                            level_up_patch(i);
                        opl_set_pitch(level_up_fnum[voice->frame >> 1], LEVEL_UP_BLOCK, 1, i);
                        voice->delay = LEVEL_UP_QUARTER_FRAMES - LEVEL_UP_GAP_FRAMES - 1;
                    }
                    else
                    {
                        opl_set_pitch(level_up_fnum[(voice->frame >> 1) - 1], LEVEL_UP_BLOCK, 0, i);
                        voice->delay = LEVEL_UP_GAP_FRAMES - 1;
                    }
                }
                else
                {
                    opl_write(0x54 + i, 63);
                    voice->sound = SFX_NONE;
                    continue;
                }
                ++voice->frame;
                break;
            case SFX_MONSTER_DAMAGE:
            case SFX_DAMAGE:
                if (voice->frame == 0)
                {
                    opl_set_pitch(damage_fnum[0], DAMAGE_BLOCK, 0, i);
                }
                else if (voice->frame <= DAMAGE_FRAMES)
                {
                    if (voice->frame == 1)
                        damage_patch(i);
                    /* Reverse the control envelope, retaining the same timbre. */
                    damage_index = voice->sound == SFX_MONSTER_DAMAGE
                        ? DAMAGE_FRAMES - voice->frame : voice->frame - 1;
                    opl_write(0x54 + i, damage_level[damage_index]);
                    opl_set_pitch(damage_fnum[damage_index], DAMAGE_BLOCK, 1, i);
                }
                else
                {
                    opl_write(0x54 + i, 63);
                    opl_set_pitch(damage_fnum[DAMAGE_FRAMES - 1], DAMAGE_BLOCK, 0, i);
                    voice->sound = SFX_NONE;
                    continue;
                }
                ++voice->frame;
                break;
            case SFX_SWORD:
                if (voice->frame == 0)
                {
                    opl_set_pitch(sword_fnum[0], SWORD_BLOCK, 0, i);
                }
                else if (voice->frame <= SWORD_FRAMES)
                {
                    if (voice->frame == 1)
                        sword_patch(i);
                    opl_write(0x54 + i, sword_level[voice->frame - 1]);
                    opl_set_pitch(sword_fnum[voice->frame - 1], SWORD_BLOCK, 1, i);
                }
                else
                {
                    opl_write(0x54 + i, 63);
                    opl_set_pitch(sword_fnum[SWORD_FRAMES - 1], SWORD_BLOCK, 0, i);
                    voice->sound = SFX_NONE;
                    continue;
                }
                ++voice->frame;
                break;
            case SFX_DOOR:
                if (voice->frame == 0)
                {
                    opl_set_pitch(DOOR_FNUM, DOOR_BLOCK, 0, i);
                }
                else if (voice->frame <= DOOR_FRAMES)
                {
                    if (voice->frame == 1)
                    {
                        door_patch(i);
                        opl_set_pitch(DOOR_FNUM, DOOR_BLOCK, 1, i);
                    }
                    /* Envelope index 0 starts on the same update as key-on. */
                    opl_write(0x54 + i, door_level[voice->frame - 1]);
                    opl_write(0x51 + i,
                              DOOR_MOD_LEVEL + mod_attenuation[(voice->frame - 1) >> 1]);
                }
                else
                {
                    opl_set_pitch(DOOR_FNUM, DOOR_BLOCK, 0, i);
                    voice->sound = SFX_NONE;
                    continue;
                }
                ++voice->frame;
                /* One update per frame: no extra skipped updates. */
                break;
            case SFX_PICKUP:
                switch (voice->frame)
                {
                    case 0:
                        opl_set_pitch(PICKUP_LOW_FNUM, PICKUP_BLOCK, 0, i);
                        /* Next update is one frame later; skip no updates. */
                        voice->delay = 0;
                        break;

                    case 1:
                        pickup_patch(i);
                        opl_set_pitch(PICKUP_LOW_FNUM, PICKUP_BLOCK, 1, i);
                        voice->delay = PICKUP_FIRST_FRAMES - 1;
                        break;

                    case 2:
                        opl_set_pitch(PICKUP_HIGH_FNUM, PICKUP_BLOCK, 1, i);
                        voice->delay = PICKUP_HOLD_FRAMES - 1;
                        break;

                    default:
                        /* Steps 3..38 apply fade levels 1..36, one per frame.
                         * Keep key-on and the modulator unchanged during fade. */
                        if (voice->frame < 3 + PICKUP_FADE_FRAMES)
                        {
                            opl_write(0x54 + i, PICKUP_LEVEL + voice->frame - 2);
                            break;
                        }
                        opl_set_pitch(PICKUP_HIGH_FNUM, PICKUP_BLOCK, 0, i);
                        voice->sound = SFX_NONE;
                        continue;
                }
                ++voice->frame;
                break;

            default:
                voice->sound = SFX_NONE;
                break;
        }
    }
}

bool is_sfx_register(uint8_t reg)
{
    uint8_t slot;
    /* OPL operator banks share the same sparse slot mapping. Avoid a
     * 26-entry search for every music packet on the 6502. */
    switch (reg & 0xE0)
    {
        case 0x20:
        case 0x40:
        case 0x60:
        case 0x80:
        case 0xE0:
            slot = reg & 0x1F;
            return slot == 0x11 || slot == 0x12 ||
                   slot == 0x14 || slot == 0x15;
    }
    return reg == 0xA7 || reg == 0xA8 ||
           reg == 0xB7 || reg == 0xB8 ||
           reg == 0xC7 || reg == 0xC8;
}

void sound_init(void)
{
    uint8_t i;
    sfx_voice_t *voice = voices;
    /* Enabling OPL resets the chip AND its XRAM register page. Configure
     * it once here, before writing the shared settings or any patches. */
    if (xreg_ria_opl(XRAM_OPL) < 0)
    {
        perror("OPL2 setup");
        return;
    }
    /* Shared chip settings belong to the game, not the music stream. */
    opl_write(0x01, 0x20); /* Enable the waveforms selected by each patch. */
    opl_write(0x08, 0x00); /* Normal note-select mode; CSM disabled. */
    opl_write(0xBD, 0x00); /* Nine melodic channels; rhythm mode disabled. */
    opl_write(0xB7, 0x00);
    opl_write(0xB8, 0x00);
    
    for (i = 0; i < SFX_CHANNEL_COUNT; i++, voice++)
    {
        voice->sound = SFX_NONE;
        voice->frame = 0;
        voice->delay = 0;
    }
}
