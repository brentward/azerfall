#include "music.h"

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <rp6502.h>

#include "../xram.h"
#include "sound.h"

#define SONG_PACKET_SIZE 4U
#define SONG_READ_CHUNK 4096U

/* Two operators per music channel, channels 0 through 6.
 * Channels 7 and 8 belong to sound effects.
 */
static const uint8_t music_volume_regs[] = {
    0x40, 0x43,
    0x41, 0x44,
    0x42, 0x45,
    0x48, 0x4B,
    0x49, 0x4C,
    0x4A, 0x4D,
    0x50, 0x53
};

static uint8_t saved_music_volumes[14];
static bool music_paused = false;
static uint8_t loop_registers[256];
static uint8_t loop_replaced_keys;

static bool is_music_register(uint8_t reg)
{
    return !is_sfx_register(reg) &&
        reg != 0x01 && reg != 0x08 && reg != 0xBD;
}

static void read_packet(uint16_t address, uint8_t *buf)
{
    uint8_t i;
    RIA.addr1 = address;
    RIA.step1 = 1;
    for (i = 0; i < SONG_PACKET_SIZE; ++i, ++buf)
        *buf = RIA.rw1;
}

static void write_music_register(uint8_t reg, uint8_t value)
{
    if (is_music_register(reg))
    {
        RIA.addr1 = XRAM_OPL + reg;
        RIA.rw1 = value;
    }
}

static void restore_music_registers(const uint8_t *registers, uint8_t replaced_keys)
{
    unsigned reg;
    const uint8_t *value = registers;
    /* Restore patches/pitches before key-on; never touch SFX channels. */
    for (reg = 0; reg < 256; ++reg, ++value)
        if (reg < 0xB0 || reg > 0xB8)
            write_music_register((uint8_t)reg, *value);
    value = registers + 0xB0;
    for (reg = 0xB0; reg <= 0xB6; ++reg, ++value)
        if (!(replaced_keys & (1U << (reg - 0xB0))))
            write_music_register((uint8_t)reg, *value);
}

static void music_stop(game_t *game)
{
    unsigned i;

    music_resume();

    game->song_bytes_remaining = 0;
    game->song_delay = 0;
    for (i = 0; i < 7; ++i)
    {
        RIA.addr1 = XRAM_OPL + 0xB0 + i;
        RIA.rw1 = 0;
    }
}

void music_init(game_t *game, const char *path, bool loop)
{
    int fd;
    int bytes;
    unsigned total_bytes = 0;
    unsigned count;
    uint8_t extra;

    music_stop(game);
    game->song_size = 0;
    game->song_loop_offset = 0;
    game->song_xram_ptr = XRAM_SONG_DATA;
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror(path);
        return;
    }

    /* Short transfers are valid; never write past the song region. */
    while (total_bytes < SONG_DATA_MAX_BYTES)
    {
        count = SONG_DATA_MAX_BYTES - total_bytes;
        if (count > SONG_READ_CHUNK)
            count = SONG_READ_CHUNK;
        bytes = read_xram(XRAM_SONG_DATA + total_bytes, count, fd);
        if (bytes < 0)
        {
            perror("Music read");
            close(fd);
            return;
        }
        if (bytes == 0)
            break;
        total_bytes += bytes;
    }

    /* Probe oversized files in CPU RAM, without touching sprite configs. */
    if (total_bytes == SONG_DATA_MAX_BYTES)
    {
        bytes = read(fd, &extra, 1);
        if (bytes != 0)
        {
            if (bytes < 0)
                perror("Music read");
            else
                puts("Music exceeds available XRAM");
            close(fd);
            return;
        }
    }
    close(fd);

    /* sound_init owns OPL setup. Re-enabling it here would erase the
     * waveform-enable bit and any active SFX instrument settings. */
    game->song_bytes_remaining = total_bytes;
    if (loop)
        game->song_size = total_bytes;
    else
        game->song_size = 0;
    printf("Music loaded: %u bytes\n", total_bytes);
}

bool music_set_loop_offset(game_t *game, uint16_t offset)
{
    uint16_t pos;
    unsigned reg;
    uint8_t *value = loop_registers;
    uint8_t buf[SONG_PACKET_SIZE];
    bool has_delay = false;

    if (offset % SONG_PACKET_SIZE != 0 || offset >= game->song_size)
        return false;
    /* Reject empty/zero-time loops and offsets beyond the first end marker. */
    for (pos = 0; pos + SONG_PACKET_SIZE <= game->song_size;
         pos += SONG_PACKET_SIZE)
    {
        read_packet(XRAM_SONG_DATA + pos, buf);
        if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0 && buf[3] == 0)
            break;
        if (pos >= offset && (buf[2] != 0 || buf[3] != 0))
            has_delay = true;
    }
    if (!has_delay || offset >= pos || pos + SONG_PACKET_SIZE > game->song_size)
        return false;
    for (reg = 0; reg < 256; ++reg, ++value)
        *value = 0;
    for (pos = 0; pos < offset; pos += SONG_PACKET_SIZE)
    {
        read_packet(XRAM_SONG_DATA + pos, buf);
        loop_registers[buf[0]] = buf[1];
    }
    game->song_loop_offset = offset;
    /* The first batch may immediately replace a saved key state. Restoring
     * that state first can briefly retrigger an old note (notably the bass).
     * Let the stream perform those key transitions exactly once instead. */
    loop_replaced_keys = 0;
    for (pos = offset; pos + SONG_PACKET_SIZE <= game->song_size;
         pos += SONG_PACKET_SIZE)
    {
        read_packet(XRAM_SONG_DATA + pos, buf);
        if (buf[0] >= 0xB0 && buf[0] <= 0xB6)
            loop_replaced_keys |= 1U << (buf[0] - 0xB0);
        if (buf[2] != 0 || buf[3] != 0)
            break;
    }
    return true;
}

void music_skip_to_frame(game_t *game, uint16_t frame)
{
    uint8_t buf[SONG_PACKET_SIZE];
    /* cc65 cannot address a 256-byte automatic array on its software stack. */
    static uint8_t registers[256];
    uint8_t *value = registers;
    unsigned reg;
    uint16_t delay;

    /* Call immediately after loading/configuring the loop, before playback. */
    if (frame == 0 || game->song_bytes_remaining == 0)
        return;
    for (reg = 0; reg < 256; ++reg, ++value)
        *value = 0;
    while (frame != 0 && game->song_bytes_remaining >= SONG_PACKET_SIZE)
    {
        read_packet(game->song_xram_ptr, buf);
        if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0 && buf[3] == 0)
            break;
        game->song_xram_ptr += SONG_PACKET_SIZE;
        game->song_bytes_remaining -= SONG_PACKET_SIZE;
        registers[buf[0]] = buf[1];
        delay = buf[2] | ((uint16_t)buf[3] << 8);
        if (delay > frame)
        {
            game->song_delay = delay - frame + 1;
            break;
        }
        frame -= delay;
    }
    /* Apply only the final state, avoiding an audible burst of skipped notes.
     * Oscillator/envelope phase cannot be reconstructed by fast-forwarding. */
    restore_music_registers(registers, 0);
}

/* RPTracker BIN packets are [register, value, delay low, delay high].
 * The delay follows the write and is measured in 60 Hz frames. */
void music_update(game_t *game)
{
    uint8_t buf[SONG_PACKET_SIZE];

    if (music_paused)
        return;

    if (game->song_bytes_remaining == 0)
        return;
    if (game->song_delay > 0)
        --game->song_delay;

    while (game->song_delay == 0)
    {
        if (game->song_bytes_remaining < SONG_PACKET_SIZE)
        {
            music_stop(game);
            return;
        }
        read_packet(game->song_xram_ptr, buf);
        game->song_xram_ptr += SONG_PACKET_SIZE;
        game->song_bytes_remaining -= SONG_PACKET_SIZE;

        if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0 && buf[3] == 0)
        {
            if (game->song_size != 0)
            {
                if (game->song_loop_offset != 0)
                    restore_music_registers(loop_registers, loop_replaced_keys);
                game->song_xram_ptr = XRAM_SONG_DATA + game->song_loop_offset;
                game->song_bytes_remaining = game->song_size - game->song_loop_offset;
                continue;
            } else 
            {
                music_stop(game);
                return;
            }
        }
        /* Preserve SFX patches and the shared settings established by sound_init.
         * RPTracker exports whole-chip resets at the start/end of the song. */
        write_music_register(buf[0], buf[1]);
        game->song_delay = buf[2] | ((uint16_t)buf[3] << 8);
    }
}

void music_pause(void)
{
    uint8_t i;
    const uint8_t *volume_reg = music_volume_regs;
    uint8_t *volume = saved_music_volumes;

    /* Don't overwrite our saved volumes if already paused. */
    if (music_paused)
        return;

    for (i = 0; i < 14; ++i, ++volume_reg, ++volume)
    {
        /* Read the current volume byte from the OPL XRAM buffer.
         * A zero step keeps the address unchanged after reading.
         */
        RIA.step1 = 0;
        RIA.addr1 = XRAM_OPL + *volume_reg;
        *volume = RIA.rw1;

        /* Lower six bits control attenuation:
         * 0 = loudest, 63 = quietest.
         * OR preserves the upper two bits.
         */
        RIA.rw1 = *volume | 0x3F;
    }

    music_paused = true;
}

void music_resume(void)
{
    uint8_t i;
    const uint8_t *volume_reg = music_volume_regs;
    uint8_t *volume = saved_music_volumes;

    if (!music_paused)
        return;

    for (i = 0; i < 14; ++i, ++volume_reg, ++volume)
    {
        RIA.step1 = 0;
        RIA.addr1 = XRAM_OPL + *volume_reg;
        RIA.rw1 = *volume;
    }

    music_paused = false;
}
