"""Exercise the OPL voice scheduler with cc65/sim65 and mock register storage."""

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class OplVoiceTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cl65") and shutil.which("sim65"),
                         "requires cl65 and sim65 on PATH")
    def test_two_voice_lifecycle(self):
        root = Path(__file__).resolve().parents[1]
        source = (root / "src/audio/sound.c").read_text()
        source = source.replace('#include <rp6502.h>', '')
        source = source.replace('#include "../xram.h"', '''
#define XRAM_OPL 0
static struct { unsigned addr1; unsigned char rw1; } RIA;
static int test_opl_setup(unsigned address)
{
    unsigned i;
    for (i = 0; i < 256; ++i) test_registers[i] = 0;
    return address == XRAM_OPL ? 0 : -1;
}
#define xreg_ria_opl test_opl_setup
''')
        source = source.replace('RIA.rw1 = value;',
                                'RIA.rw1 = value; test_registers[reg] = value;')
        source = 'static unsigned char test_registers[256];\n' + source
        harness = r'''
#include <assert.h>
int main(void)
{
    unsigned i;
    sound_init();
    assert(test_registers[0x01] == 0x20);
    /* Exhaustive check against the two reserved channels' register map. */
    for (i = 0; i < 256; ++i)
    {
        unsigned char expected =
            i == 0x31 || i == 0x34 || i == 0x32 || i == 0x35 ||
            i == 0x51 || i == 0x54 || i == 0x52 || i == 0x55 ||
            i == 0x71 || i == 0x74 || i == 0x72 || i == 0x75 ||
            i == 0x91 || i == 0x94 || i == 0x92 || i == 0x95 ||
            i == 0xF1 || i == 0xF4 || i == 0xF2 || i == 0xF5 ||
            i == 0xA7 || i == 0xB7 || i == 0xC7 ||
            i == 0xA8 || i == 0xB8 || i == 0xC8;
        assert(is_sfx_register((unsigned char)i) == expected);
    }
    sound_play(SFX_NONE);
    assert(voices[0].sound == SFX_NONE);
    sound_play(SFX_DOOR);
    sound_play(SFX_PICKUP);
    sound_play(SFX_DOOR); /* Busy: neither slot may be replaced. */
    assert(voices[0].sound == SFX_DOOR);
    assert(voices[1].sound == SFX_PICKUP);
    sound_update(); /* t=0: key-off */
    assert(voices[0].frame == 1 && voices[1].frame == 1);
    assert(RIA.addr1 == 0xB8 && !(RIA.rw1 & 0x20));
    sound_update(); /* t=1: patch and key-on */
    assert(voices[0].delay == 0 && voices[1].frame == 2);
    assert(RIA.addr1 == 0xB8 && (RIA.rw1 & 0x20));
    for (i = 2; i <= PICKUP_FIRST_FRAMES; ++i) sound_update();
    assert(voices[1].frame == 2);
    sound_update(); /* t=7: high note, no intervening key-off */
    assert(voices[1].frame == 3);
    assert(RIA.addr1 == 0xB8 && (RIA.rw1 & 0x20));
    for (i = 1; i < PICKUP_HOLD_FRAMES; ++i) sound_update();
    assert(voices[0].sound == SFX_DOOR);
    for (i = 1; i <= PICKUP_FADE_FRAMES; ++i)
    {
        sound_update();
        assert(voices[1].sound == SFX_PICKUP);
        assert(RIA.addr1 == 0x55 && RIA.rw1 == PICKUP_LEVEL + i);
    }
    sound_update(); /* One frame after the last fade write: release. */
    assert(voices[0].sound == SFX_NONE);
    assert(voices[1].sound == SFX_NONE);
    assert(RIA.addr1 == 0xB8 && !(RIA.rw1 & 0x20));
    sound_play(SFX_DOOR);
    assert(voices[0].sound == SFX_DOOR && voices[0].frame == 0);
    sound_init();
    assert(voices[0].sound == SFX_NONE && voices[1].sound == SFX_NONE);
    /* The door's 48 envelope entries must each last exactly one update. */
    sound_play(SFX_DOOR);
    sound_update();
    assert(!(test_registers[0xB7] & 0x20));
    for (i = 0; i < DOOR_FRAMES; ++i)
    {
        sound_update();
        assert(voices[0].sound == SFX_DOOR);
        assert(test_registers[0xB7] & 0x20);
        assert(test_registers[0x54] == door_level[i]);
        assert(test_registers[0x51] == DOOR_MOD_LEVEL + mod_attenuation[i >> 1]);
    }
    sound_update();
    assert(voices[0].sound == SFX_NONE);
    assert(!(test_registers[0xB7] & 0x20));
    /* Slash sweeps on either channel, ends promptly, and can be retriggered
     * without interrupting a longer effect on the other channel. */
    sound_play(SFX_DOOR);
    sound_play(SFX_SWORD);
    sound_play(SFX_PICKUP);
    assert(voices[0].sound == SFX_DOOR && voices[1].sound == SFX_SWORD);
    sound_update();
    assert(!(test_registers[0xB8] & 0x20));
    for (i = 0; i < SWORD_FRAMES; ++i)
    {
        sound_update();
        assert(voices[1].sound == SFX_SWORD);
        assert(test_registers[0xB8] ==
               (0x20 | (SWORD_BLOCK << 2) | (sword_fnum[i] >> 8)));
        assert(test_registers[0xA8] == (unsigned char)sword_fnum[i]);
        assert(test_registers[0x55] == sword_level[i]);
    }
    sound_update();
    assert(voices[0].sound == SFX_DOOR && voices[1].sound == SFX_NONE);
    assert(!(test_registers[0xB8] & 0x20));
    assert(test_registers[0x55] == 63);
    sound_play(SFX_SWORD);
    assert(voices[1].sound == SFX_SWORD && voices[1].frame == 0);
    sound_init();
    sound_play(SFX_SWORD);
    sound_update();
    sound_update();
    assert(test_registers[0xB7] & 0x20);
    assert(test_registers[0xC7] == 0x0E);
    assert(test_registers[0x54] == sword_level[0]);
    /* Damage must preempt a busy second voice without changing the first. */
    sound_init();
    sound_play(SFX_DOOR);
    sound_play(SFX_PICKUP);
    sound_update();
    sound_update();
    sound_play(SFX_DAMAGE);
    assert(voices[0].sound == SFX_DOOR);
    assert(voices[1].sound == SFX_DAMAGE && voices[1].frame == 0);
    sound_update();
    assert(!(test_registers[0xB8] & 0x20));
    for (i = 0; i < DAMAGE_FRAMES; ++i)
    {
        sound_update();
        assert(voices[1].sound == SFX_DAMAGE);
        assert(test_registers[0xB8] ==
               (0x20 | (DAMAGE_BLOCK << 2) | (damage_fnum[i] >> 8)));
        assert(test_registers[0xA8] == (unsigned char)damage_fnum[i]);
        assert(test_registers[0x55] == damage_level[i]);
    }
    sound_update();
    assert(voices[0].sound == SFX_DOOR && voices[1].sound == SFX_NONE);
    assert(!(test_registers[0xB8] & 0x20));
    assert(test_registers[0x55] == 63);
    /* Retrigger on the first free channel after releasing both voices. */
    sound_init();
    sound_play(SFX_DAMAGE);
    sound_update();
    sound_update();
    assert(voices[0].sound == SFX_DAMAGE);
    assert(test_registers[0xB7] & 0x20);
    assert(test_registers[0x54] == damage_level[0]);
    /* Monster hits reverse both pitch and attenuation, alongside a slash. */
    sound_init();
    sound_play(SFX_SWORD);
    sound_play(SFX_MONSTER_DAMAGE);
    sound_update();
    assert(!(test_registers[0xB8] & 0x20));
    for (i = 0; i < DAMAGE_FRAMES; ++i)
    {
        sound_update();
        assert(voices[1].sound == SFX_MONSTER_DAMAGE);
        assert(test_registers[0xB8] ==
               (0x20 | (DAMAGE_BLOCK << 2) |
                (damage_fnum[DAMAGE_FRAMES - 1 - i] >> 8)));
        assert(test_registers[0xA8] ==
               (unsigned char)damage_fnum[DAMAGE_FRAMES - 1 - i]);
        assert(test_registers[0x55] == damage_level[DAMAGE_FRAMES - 1 - i]);
    }
    sound_update();
    assert(voices[1].sound == SFX_NONE);
    assert(!(test_registers[0xB8] & 0x20));
    assert(test_registers[0x55] == 63);
    /* The four flute notes start exactly one quarter note apart, even
     * when the level-up request must replace an occupied second voice. */
    sound_init();
    sound_play(SFX_DOOR);
    sound_play(SFX_MONSTER_DAMAGE);
    sound_play(SFX_LEVEL_UP);
    assert(voices[0].sound == SFX_DOOR);
    assert(voices[1].sound == SFX_LEVEL_UP);
    sound_update();
    assert(!(test_registers[0xB8] & 0x20));
    for (i = 0; i < 4 * LEVEL_UP_QUARTER_FRAMES; ++i)
    {
        unsigned note = i / LEVEL_UP_QUARTER_FRAMES;
        unsigned phase = i % LEVEL_UP_QUARTER_FRAMES;
        sound_update();
        assert(voices[1].sound == SFX_LEVEL_UP);
        assert(test_registers[0xA8] == (unsigned char)level_up_fnum[note]);
        assert(test_registers[0xB8] ==
               ((phase < LEVEL_UP_QUARTER_FRAMES - LEVEL_UP_GAP_FRAMES ? 0x20 : 0) |
                (LEVEL_UP_BLOCK << 2) | (level_up_fnum[note] >> 8)));
        assert(test_registers[0xF2] == 0 && test_registers[0xF5] == 0);
        assert(test_registers[0xC8] == 0);
    }
    sound_update();
    assert(voices[1].sound == SFX_NONE);
    assert(test_registers[0x55] == 63);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder)
            (path / "sound.h").write_text((root / "src/audio/sound.h").read_text())
            (path / "test.c").write_text(source + harness)
            subprocess.run(["cl65", "-t", "sim6502", "-o", "test", "test.c"],
                           cwd=path, check=True, capture_output=True, text=True)
            subprocess.run(["sim65", "test"], cwd=path, check=True,
                           capture_output=True, text=True)


if __name__ == "__main__":
    unittest.main()
