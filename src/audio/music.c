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

static void music_stop(Game *game)
{
    unsigned i;

    game->song_bytes_remaining = 0;
    game->song_delay = 0;
    for (i = 0; i < 7; ++i)
    {
        RIA.addr1 = XRAM_OPL + 0xB0 + i;
        RIA.rw1 = 0;
    }
}

void music_init(Game *game, const char *path, bool loop)
{
    int fd;
    int bytes;
    unsigned total_bytes = 0;
    unsigned count;
    uint8_t extra;

    music_stop(game);
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

    if (xreg_ria_opl(XRAM_OPL) < 0)
    {
        perror("OPL2 setup");
        return;
    }
    game->song_bytes_remaining = total_bytes;
    if (loop)
        game->song_size = total_bytes;
    else
        game->song_size = 0;
    printf("Music loaded: %u bytes\n", total_bytes);
}

/* RPTracker BIN packets are [register, value, delay low, delay high].
 * The delay follows the write and is measured in 60 Hz frames. */
void music_update(Game *game)
{
    uint8_t buf[SONG_PACKET_SIZE];
    uint8_t i;

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
        RIA.addr1 = game->song_xram_ptr;
        RIA.step1 = 1;
        for (i = 0; i < SONG_PACKET_SIZE; ++i)
            buf[i] = RIA.rw1;
        game->song_xram_ptr += SONG_PACKET_SIZE;
        game->song_bytes_remaining -= SONG_PACKET_SIZE;

        if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0 && buf[3] == 0)
        {
            if (game->song_size != 0)
            {
                game->song_xram_ptr = XRAM_SONG_DATA;
                game->song_bytes_remaining = game->song_size;
                continue;
            } else 
            {
                music_stop(game);
                return;
            }
        }
        /* Preserve SFX patches and the shared settings established by sound_init.
         * RPTracker exports whole-chip resets at the start/end of the song. */
        if (!is_sfx_register(buf[0]) &&
            buf[0] != 0x01 && buf[0] != 0x08 && buf[0] != 0xBD)
        {
            RIA.addr1 = XRAM_OPL + buf[0];
            RIA.rw1 = buf[1];
        }
        game->song_delay = buf[2] | ((uint16_t)buf[3] << 8);
    }
}
