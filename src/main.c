#include <rp6502.h>
#include <stdio.h>
#include <stdbool.h>
#include "game.h"



int main(void)
{
    unsigned char frame;

    puts("AZERFALL RP6502 PORT");
    puts("runtime online");
    puts("next milestone: video and input");

    init_game();

    frame = RIA.vsync;

    while (true)
    {
        while (RIA.vsync == frame)
        {
        }
        frame = RIA.vsync;

        update_input();
        update_game();
        update_animation();
        draw();
    }
}
