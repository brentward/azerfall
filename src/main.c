#include <stdio.h>
#include <stdbool.h>

#include <rp6502.h>

#include "game\game.h"

int main(void)
{
    unsigned char frame;

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
        
        draw();
        
    }
}
