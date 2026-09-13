#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

// GAMEPAD
#define GAMEPAD_INPUT 0xFF80U // 40 bytes of gamepad data

// USB KEYBOARD
#define KEYBOARD_INPUT 0xFF10U // KEYBOARD_BYTES (32 bytes, 256 bits) of key press bitmask data

typedef struct
{
    bool up_pressed;
    bool down_pressed;
    bool left_pressed;
    bool right_pressed; 
    bool pause_pressed;
    bool background_reload_pressed; // Temporary R-key graphics diagnostic.
} InputState;

extern InputState input_state;

void input_init(void);
void input_update(void);

#endif
