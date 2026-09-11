#ifndef INPUT_H
#define INPUT_H

#include <rp6502.h>
#include <stdint.h>
#include <stdbool.h>

// GAMEPAD
#define GAMEPAD_INPUT 0xFF80U // 40 bytes of gamepad data

// USB KEYBOARD
#define KEYBOARD_INPUT 0xFF10U // KEYBOARD_BYTES (32 bytes, 256 bits) of key press bitmask data
// 256 bytes HID code max, stored in 32 uint8
#define KEYBOARD_BYTES 32
// keystates[code>>3] gets contents from correct byte in array
// 1 << (code&7) moves a 1 into proper position to mask with byte contents
// final & gives 1 if key is pressed, 0 if not
#define key(code) (keystates[code >> 3] & (1 << (code & 7)))
extern uint8_t keystates[KEYBOARD_BYTES];

typedef struct
{
    bool up_pressed;
    bool down_pressed;
    bool left_pressed;
    bool right_pressed; 
} InputState;

extern InputState input_state;

void input_init(void);
void input_update(void);

#endif