#include <stdio.h>
#include <stdbool.h>

#include <rp6502.h>

#include "game\game.h"

int main(void)
{
    unsigned char frame;

    puts("AZERFALL RP6502 PORT");
    puts("Video and input test");
    puts("next milestone: core runtime and map renderer");

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
