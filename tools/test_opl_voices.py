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
