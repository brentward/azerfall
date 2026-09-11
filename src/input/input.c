#include "input.h"
#include "../input/usb_hid_keys.h"

InputState input_state = {0};

uint8_t keystates[KEYBOARD_BYTES] = {0};


void input_init(void)
{
    int i;

    // Start with no input before enabling live updates from the RIA.
    RIA.addr0 = KEYBOARD_INPUT;
    RIA.step0 = 1;
    for (i = 0; i < KEYBOARD_BYTES; i++)
        RIA.rw0 = (i == 0) ? 1 : 0;

    RIA.addr0 = GAMEPAD_INPUT;
    for (i = 0; i < 40; i++)
        RIA.rw0 = 0;

    xreg_ria_keyboard(KEYBOARD_INPUT);
    xreg_ria_gamepad(GAMEPAD_INPUT);
}


#define GAMEPAD_SIZE 10         // bytes per gamepad in XRAM
#define GAMEPAD_CONNECTED 0x80  // byte 0 bit 7
#define GAMEPAD_BTN_START 0x08  // byte 3 bit 3
#define GAMEPAD_BTN_SELECT 0x04 // byte 3 bit 2

void input_update(void)
{
    int bits;
    unsigned char dpad;
    unsigned char sticks;
    int i;

    input_state.up_pressed = false;
    input_state.down_pressed = false;
    input_state.left_pressed = false;
    input_state.right_pressed = false;

    RIA.addr1 = KEYBOARD_INPUT;
    RIA.step1 = 1;
    for (i = 0; i < KEYBOARD_BYTES; i++)
        keystates[i] = RIA.rw1;

    RIA.addr1 = GAMEPAD_INPUT;
    // byte 0 = dpad, byte 1 = sticks: merge for direction
    dpad = RIA.rw1;
    sticks = RIA.rw1;
    bits = (dpad & GAMEPAD_CONNECTED) ? ((dpad | sticks) & 0x0F) : 0;
    if ((bits & 0x1) && !(bits & 0x2))
        input_state.up_pressed = true;
    if ((bits & 0x2) && !(bits & 0x1))
        input_state.down_pressed = true;
    if ((bits & 0x4) && !(bits & 0x8))
        input_state.left_pressed = true;
    if ((bits & 0x8) && !(bits & 0x4))
        input_state.right_pressed = true;
    // byte 2 = BTN0: A(0), B(1), X(3), Y(4) for fire
    // if (RIA.rw1 & 0x1B)
    //     Input.shoot = true;

    if (!(keystates[0] & 1)) // any key pressed?
    {
        if (key(KEY_W) && !key(KEY_S))
            input_state.up_pressed = true;
        if (key(KEY_S) && !key(KEY_W))
            input_state.down_pressed = true;
        if (key(KEY_A) && !key(KEY_D))
            input_state.left_pressed = true;
        if (key(KEY_D) && !key(KEY_A))
            input_state.right_pressed = true;
    }
}
