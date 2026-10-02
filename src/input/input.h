#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool up_pressed;
    bool down_pressed;
    bool left_pressed;
    bool right_pressed; 
    bool interact_pressed; /* New press only; consumed after an NPC interaction. */
    bool pause_pressed;
    bool background_reload_pressed;
    bool srand_seeded;
    uint16_t counter;
} input_state_t;

extern input_state_t input_state;

void input_init(void);
void input_update(void);

#endif
