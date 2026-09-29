"""Run the actual C audio logic under sim65 with mocked device/file access."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def run_c(source, headers):
    with tempfile.TemporaryDirectory() as folder:
        path = Path(folder)
        for name, content in headers.items():
            (path / name).write_text(content)
        (path / "test.c").write_text(source)
        for command in (["cl65", "-t", "sim6502", "-o", "test", "test.c"],
                        ["sim65", "test"]):
            result = subprocess.run(command, cwd=path, capture_output=True, text=True, timeout=30)
            if result.returncode:
                raise AssertionError(result.stdout + result.stderr)


@unittest.skipUnless(shutil.which("cl65") and shutil.which("sim65"), "requires cc65/sim65")
class MusicRuntimeTests(unittest.TestCase):
    def test_initialization_reload_loop_and_channel_isolation(self):
        sound = (ROOT / "src/audio/sound.c").read_text()
        music = (ROOT / "src/audio/music.c").read_text()
        for include in ('#include <rp6502.h>', '#include "../xram.h"'):
            sound = sound.replace(include, "")
            music = music.replace(include, "")
        # Replace only hardware access; retain player/init/filter logic.
        sound = sound.replace("RIA.rw1 = value;", "test_registers[reg] = value;")
        music = music.replace("RIA.rw1 = value;", "test_music_write(reg, value);")
        music = music.replace("RIA.rw1 = 0;", "test_registers[RIA.addr1] = 0;")
        music = re.sub(r"static void read_packet\([^}]+\}",
                       "static void read_packet(uint16_t address, uint8_t *buf) "
                       "{ memcpy(buf, test_song + address - XRAM_SONG_DATA, 4); }",
                       music, count=1)
        # The target's file API is stubbed below, not its libc declarations.
        music = music.replace("open(path, O_RDONLY)", "test_open(path, O_RDONLY)")
        music = music.replace("close(fd)", "test_close(fd)")
        music = music.replace("read(fd, &extra, 1)", "test_read(fd, &extra, 1)")
        prelude = r'''
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define XRAM_OPL 0
#define XRAM_SONG_DATA 256
#define SONG_DATA_MAX_BYTES 4096
static unsigned char test_registers[256];
static struct { unsigned addr1; unsigned char step1, rw1; } RIA;
static unsigned setups, loaded;
static unsigned music_key_ons;
static void test_music_write(unsigned char reg, unsigned char value)
{
    test_registers[reg] = value;
    if (reg == 0xB0 && (value & 0x20)) ++music_key_ons;
}
static const unsigned char test_song[] = {
    0x01, 0, 0, 0, 0x01, 0x20, 0, 0,
    0xB0, 0x20, 0, 0, /* Held intro note in the loop snapshot. */
    0xE0, 2, 0, 0, 0xB0, 0x20, 3, 0,
    0xB7, 0x20, 0, 0, 0xB0, 0, 2, 0,
    0xFF, 0xFF, 0, 0
};
static int xreg_ria_opl(unsigned address)
{
    assert(address == XRAM_OPL);
    ++setups;
    memset(test_registers, 0, sizeof test_registers);
    return 0;
}
static int test_open(const char *path, int flags) { loaded = 0; return 1; }
static int test_close(int fd) { return 0; }
static int test_read(int fd, void *buf, unsigned count) { return 0; }
static int read_xram(unsigned address, unsigned count, int fd)
{
    if (loaded) return 0;
    loaded = 1;
    return sizeof test_song;
}
'''
        harness = r'''
int main(void)
{
    Game game;
    unsigned i;
    memset(&game, 0, sizeof game);
    sound_init();
    assert(setups == 1 && test_registers[1] == 0x20);
    test_registers[0xB7] = 0x35; /* A sounding SFX voice. */
    test_registers[0xF1] = 3;
    music_init(&game, "song", true);
    assert(setups == 1 && test_registers[1] == 0x20);
    assert(music_set_loop_offset(&game, 12));
    music_update(&game); /* First write, followed by three frames. */
    assert(test_registers[0xE0] == 2 && test_registers[0xB0] == 0x20);
    music_update(&game);
    music_update(&game);
    assert(test_registers[0xB0] == 0x20);
    music_update(&game);
    assert(test_registers[0xB0] == 0);
    music_update(&game);
    music_key_ons = 0;
    music_update(&game); /* Loop and reapply the patch. */
    assert(music_key_ons == 1); /* No extra key-on from the old intro note. */
    assert(test_registers[0xB0] == 0x20 && test_registers[0xE0] == 2);
    assert(test_registers[1] == 0x20);
    assert(test_registers[0xB7] == 0x35 && test_registers[0xF1] == 3);
    music_init(&game, "song", true); /* Loading another song must not reset OPL. */
    assert(setups == 1 && test_registers[1] == 0x20);
    assert(test_registers[0xB7] == 0x35 && test_registers[0xF1] == 3);
    return 0;
}
'''
        run_c(prelude + sound + music + harness, {
            "sound.h": (ROOT / "src/audio/sound.h").read_text(),
            "music.h": (ROOT / "src/audio/music.h").read_text().replace("../game/game.h", "game.h"),
            "game.h": (ROOT / "src/game/game.h").read_text(),
        })

    def test_main_draws_at_fresh_vsync_before_audio_and_preserves_clock(self):
        source = (ROOT / "src/main.c").read_text()
        source = re.sub(r'^#include "[^\n]+"', '', source, flags=re.MULTILINE)
        source = source.replace('#include <rp6502.h>', '')
        source = source.replace('RIA.vsync', 'test_vsync()')
        prelude = r'''
#include <assert.h>
#include <stdlib.h>
#define XRAM_USED 0
#define XRAM_FREE 65535U
static unsigned char clock_frame, iteration, reads, vsync_reads, drawn;
static const unsigned char jumps[] = {1, 3, 251, 1};
static unsigned char test_vsync(void)
{
    /* A fresh VSYNC arrives on the first poll after sampling the fence. */
    if (++vsync_reads == 2) ++clock_frame;
    return clock_frame;
}
static void game_init(void) {}
static void game_update(void)
{
    assert(iteration < 4);
    clock_frame += jumps[iteration] - 1; /* Work may cross multiple VSYNCs. */
    vsync_reads = 0;
    drawn = 0;
}
static void audio_update(unsigned char elapsed)
{
    assert(elapsed == jumps[iteration]);
    assert(drawn); /* Music must not delay the camera/sprite writes. */
    ++reads;
    if (++iteration == 4) { assert(reads == 4); exit(0); }
}
static void timed_update(void)
{
    assert(vsync_reads == 3); /* Fence, fresh edge, timestamp: never mid-frame. */
    drawn = 1;
}
'''
        # Verify visual scheduling and audio catch-up independently, including wrap.
        run_c(prelude + source, {})


if __name__ == "__main__":
    unittest.main()
