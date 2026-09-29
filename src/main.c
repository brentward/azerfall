#include <stdio.h>
#include <stdbool.h>

#include <rp6502.h>

#include "xram.h"
#include "game\game.h"

int main(void)
{
    unsigned char frame;
    unsigned char now;
    unsigned char audio_frame;

    printf("XRAM TOTAL: 65536 bytes\n");
    printf("XRAM USED: %u bytes\n", XRAM_USED);
    printf("XRAM FREE: %u bytes\n", XRAM_FREE);

    puts("AZERFALL RP6502");
    puts("Test: Player and interaction");
    puts("Next milestone: World entities and progression");

    game_init();

    audio_frame = RIA.vsync;

    while (true)
    {
        game_update();
        /* Always wait for a fresh edge: game/audio work may have crossed
         * VSYNC, in which case drawing immediately would tear mid-scan. */
        frame = RIA.vsync;
        while (RIA.vsync == frame)
        {
        }
        now = RIA.vsync;
        /* Publish camera and sprite positions first, while still at the
         * start of the display frame. Audio batches have variable cost. */
        timed_update();
        /* Unlike the drawing fence, this timestamp persists across work
         * and waits so missed frames do not slow down the music. */
        audio_update((unsigned char)(now - audio_frame));
        audio_frame = now;
    }
}
