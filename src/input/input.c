#include "input.h"

#include <stdint.h>

#include <rp6502.h>

#include "../input/usb_hid_keys.h"

#define KEYBOARD_BYTES 32
// keystates[code>>3] gets contents from correct byte in array
// 1 << (code&7) moves a 1 into proper position to mask with byte contents
// final & gives 1 if key is pressed, 0 if not
#define key(code) (keystates[code >> 3] & (1 << (code & 7)))

InputState input_state = {0};

static uint8_t keystates[KEYBOARD_BYTES] = {0};
static bool pause_was_down = false;
static bool reload_was_down = false;

void input_init(void)
{
    int i;

    pause_was_down = false;
    reload_was_down = false;

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
    unsigned char buttons;
    bool pause_down = false;
    bool reload_down = false;
    int i;

    input_state.up_pressed = false;
    input_state.down_pressed = false;
    input_state.left_pressed = false;
    input_state.right_pressed = false;
    input_state.pause_pressed = false;

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

    // Read BTN1 directly: the commented-out BTN0 read does not advance RIA.
    RIA.addr1 = GAMEPAD_INPUT + 3;
    buttons = RIA.rw1;
    pause_down = (dpad & GAMEPAD_CONNECTED) && (buttons & GAMEPAD_BTN_START);

    if (!(keystates[0] & 1)) // any key pressed?
    {
        
        input_state.up_pressed = ((key(KEY_W) || key(KEY_UP)) != 0);
        input_state.down_pressed = ((key(KEY_S) || key(KEY_DOWN)) != 0);
        input_state.left_pressed = ((key(KEY_A) || key(KEY_LEFT)) != 0);
        input_state.right_pressed = ((key(KEY_D) || key(KEY_RIGHT)) != 0);
        pause_down = pause_down || (key(KEY_P) != 0);
        reload_down = key(KEY_R) != 0;
    }

    // Toggle only on a new press; holding P or Start must not repeat.
    input_state.pause_pressed = pause_down && !pause_was_down;
    pause_was_down = pause_down;
    input_state.background_reload_pressed = reload_down && !reload_was_down;
    reload_was_down = reload_down;

}
