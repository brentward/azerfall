/* Run with sim65; tests channel timing/allocation, not PSG audio synthesis. */
#include <assert.h>
#include <stdio.h>
#include "../src/audio/sound.h"

TestRIA test_ria;
static int setup_result;

int xreg(int device, int channel, int address, unsigned value)
{
    assert(device == 0 && channel == 1 && address == 0);
    assert(value == XRAM_SOUND_CONFIGS);
    return setup_result;
}

static const uint8_t effect[] = {
    0x28, 0x05, 128, 0, 0, 0x10, 1, 0,
    0x50, 0x05, 128, 0, 0, 0x10, 1, 0
};

int main(void)
{
    unsigned i;
    assert(sound_play_effect(effect, 2) == 0xFFFF);
    sound_init(XRAM_SOUND_CONFIGS);
    assert(sound_play_effect(effect, 0) == 0xFFFF);
    for (i = 0; i < 8; ++i)
        assert(sound_play_effect(effect, 2) == XRAM_SOUND_CONFIGS + i * 8);
    assert(sound_play_effect(effect, 2) == 0xFFFF);
    sound_update(); /* second frame */
    assert(sound_play_effect(effect, 2) == 0xFFFF);
    sound_update(); /* gate off, retain channel during release */
    assert(RIA.addr0 == XRAM_SOUND_CONFIGS + 7 * 8 + 6);
    assert(RIA.rw0 == 0);
    assert(sound_play_effect(effect, 2) == 0xFFFF);
    sound_update(); /* release completed */
    assert(sound_play_effect(effect, 2) == XRAM_SOUND_CONFIGS);

    sound_init(XRAM_SOUND_CONFIGS);
    for (i = 0; i < 8; ++i)
        assert(sound_play(1320, 2, 2, 128, 0, 0, 0x10, -63) != 0xFFFF);
    sound_update();
    sound_update();
    assert(RIA.rw0 == 130); /* preserve left pan, clear only gate */
    sound_update();
    assert(sound_play_effect(effect, 2) == 0xFFFF);
    sound_update();
    assert(sound_play_effect(effect, 2) == XRAM_SOUND_CONFIGS);

    setup_result = -1;
    sound_init(XRAM_SOUND_CONFIGS);
    assert(sound_play_effect(effect, 2) == 0xFFFF);
    puts("Sound channel lifecycle tests passed");
    return 0;
}
