#include <stdio.h>
#include <stdbool.h>

#include <rp6502.h>

#include "xram.h"
#include "game\game.h"

int main(void)
{
    unsigned char frame;

    printf("XRAM TOTAL: 65536 bytes\n");
    printf("XRAM USED: %u bytes\n", XRAM_USED);
    printf("XRAM FREE: %u bytes\n", XRAM_FREE);

    puts("AZERFALL RP6502");
    puts("Test: Player and interaction");
    puts("Next milestone: World entities and progression");

    game_init();

    frame = RIA.vsync;

    while (true)
    {
        game_update();
        frame = RIA.vsync;
        while (RIA.vsync == frame)
        {
        }
        timed_update();
    }
}
