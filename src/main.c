#include <stdio.h>
#include <stdbool.h>

#include <rp6502.h>

#include "xram.h"
#include "game\game.h"

unsigned int logic_crossings = 0;
unsigned int draw_crossings = 0;
unsigned int audio_crossings = 0;

int main(void)
{
    unsigned char frame;
    unsigned char now;
    unsigned char audio_frame;
    unsigned char start;
    unsigned char elapsed;
    unsigned int work_one_frame = 0;
    unsigned int work_multiple_frames = 0;
    unsigned int work_extra_frames = 0;

    printf("XRAM TOTAL: 65536 bytes\n");
    printf("XRAM USED: %u bytes\n", XRAM_USED);
    printf("XRAM FREE: %u bytes\n", XRAM_FREE);

    puts("AZERFALL RP6502");
    puts("Test: Player and interaction");
    puts("Next milestone: World entities and progression");

    game_init();

    audio_frame = RIA.vsync;
    now = audio_frame;

    while (true)
    {
        // start = RIA.vsync;
        game_update();
        // // logic_crossings += (unsigned char)(RIA.vsync - start);
        // frame = RIA.vsync;
        // elapsed = (unsigned char)(frame - now);
        // if (elapsed > 1)
        //     work_extra_frames += elapsed - 1;
        // if (elapsed == 1)
        //     work_one_frame++;
        // else if (elapsed > 1)
        //     work_multiple_frames++;
        /* Always wait for a fresh edge: game/audio work may have crossed
         * VSYNC, in which case drawing immediately would tear mid-scan. */
        frame = RIA.vsync;
        while (RIA.vsync == frame)
        {
        }
        now = RIA.vsync;
        /* Publish camera and sprite positions first, while still at the
         * start of the display frame. Audio batches have variable cost. */
        // start = RIA.vsync;
        timed_update();
        // draw_crossings += (unsigned char)(RIA.vsync - start);
        /* Unlike the drawing fence, this timestamp persists across work
         * and waits so missed frames do not slow down the music. */
        // start = RIA.vsync;
        audio_update((unsigned char)(now - audio_frame));
        // audio_crossings += (unsigned char)(RIA.vsync - start);
        audio_frame = now;
    }
}
