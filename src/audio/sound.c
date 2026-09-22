#include "sound.h"

#include <stddef.h>
#include <stdio.h>
#include "../../generated/sfx_example.h"

#define PSG_CHANNEL_BYTES 8U

static struct channel
{
    uint16_t xaddr;
    uint16_t frames_left;
    const uint8_t *frames;
    uint8_t duration;
    uint8_t release;
    uint8_t pan_gate;
    bool active;
    bool releasing;
} psg_channels[PSG_CHANNELS];

static bool psg_ready;

static void write_frame(struct channel *channel, const uint8_t *frame)
{
    uint8_t i;
    RIA.addr0 = channel->xaddr;
    RIA.step0 = 1;
    for (i = 0; i < PSG_CHANNEL_BYTES; ++i)
        RIA.rw0 = frame[i];
    channel->pan_gate = frame[6];
}

static struct channel *free_channel(void)
{
    uint8_t i;
    if (!psg_ready)
        return NULL;
    for (i = 0; i < PSG_CHANNELS; ++i)
        if (!psg_channels[i].active)
            return &psg_channels[i];
    return NULL;
}

void sound_init(uint16_t xram_address)
{
    unsigned i;
    psg_ready = false;
    RIA.addr0 = xram_address;
    RIA.step0 = 1;
    for (i = 0; i < PSG_CHANNELS * PSG_CHANNEL_BYTES; ++i)
        RIA.rw0 = 0;
    for (i = 0; i < PSG_CHANNELS; ++i)
    {
        psg_channels[i].xaddr = xram_address + i * PSG_CHANNEL_BYTES;
        psg_channels[i].active = false;
    }
    if (xreg_ria_psg(xram_address) < 0)
    {
        perror("PSG setup");
        return;
    }
    psg_ready = true;
}

uint16_t sound_play(uint16_t freq, uint8_t duration, uint8_t release,
                    uint8_t duty, uint8_t vol_attack, uint8_t vol_decay,
                    uint8_t wave_release, int8_t pan)
{
    uint8_t frame[PSG_CHANNEL_BYTES];
    struct channel *channel = free_channel();
    if (!channel)
        return 0xFFFF;
    frame[0] = freq & 255;
    frame[1] = freq >> 8;
    frame[2] = duty;
    frame[3] = vol_attack;
    frame[4] = vol_decay;
    frame[5] = wave_release;
    frame[6] = PSG_PAN(pan) | PSG_GATE;
    frame[7] = 0;
    channel->duration = duration ? duration : 1;
    channel->release = release ? release : 1;
    channel->frames_left = 0;
    channel->frames = NULL;
    channel->releasing = false;
    channel->active = true;
    write_frame(channel, frame);
    return channel->xaddr;
}

uint16_t sound_play_effect(const uint8_t *frames, uint16_t frame_count)
{
    struct channel *channel = free_channel();
    if (!channel || !frames || !frame_count)
        return 0xFFFF;
    channel->duration = 1;
    channel->release = 1; /* Generated frames use the fastest (6 ms) release. */
    channel->frames_left = frame_count - 1;
    channel->frames = frames + PSG_CHANNEL_BYTES;
    channel->releasing = false;
    channel->active = true;
    write_frame(channel, frames);
    return channel->xaddr;
}

/* Call once per 60 Hz tick. A channel remains reserved during release. */
void sound_update(void)
{
    uint8_t i;
    struct channel *channel;
    for (i = 0; i < PSG_CHANNELS; ++i)
    {
        channel = &psg_channels[i];
        if (!channel->active)
            continue;
        if (channel->releasing)
        {
            if (--channel->release == 0)
                channel->active = false;
            continue;
        }
        if (--channel->duration != 0)
            continue;
        if (channel->frames_left)
        {
            write_frame(channel, channel->frames);
            channel->frames += PSG_CHANNEL_BYTES;
            --channel->frames_left;
            channel->duration = 1;
        }
        else
        {
            RIA.addr0 = channel->xaddr + 6;
            RIA.step0 = 0;
            RIA.rw0 = channel->pan_gate & 0xFE;
            channel->releasing = true;
        }
    }
}

/* Example export; replace with a different generated effect as desired. */
bool sound_play_door_open(void)
{
    return sound_play_effect(sfx_example, SFX_EXAMPLE_FRAMES) != 0xFFFF;
}
