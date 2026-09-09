#include <rp6502.h>
#include <stdio.h>
#include <stdbool.h>
#include "game.h"



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
        while (RIA.vsync == frame)
        {
        }
        frame = RIA.vsync;

        game_update();
    }
}
