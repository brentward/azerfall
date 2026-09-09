#include "game.h"
#include "player.h"
#include "usb_hid_keys.h"

InputState Input = {0};

uint8_t keystates[KEYBOARD_BYTES] = {0};

int player_x = 160;
int player_y = 90;
int player_speed = 4;


void game_update(void)
{
        input_update();
        animation_update();
}

void animation_update(void)
{
    player_update();
}

void game_init(void)
{
    background_init();
    player_init();
    input_init();
}

void background_init(void)
{
    int i;
    xreg_vga_canvas(4);
    xram0_struct_set(0xFF00, vga_mode2_config_t, x_wrap, true);
    xram0_struct_set(0xFF00, vga_mode2_config_t, y_wrap, true);
    xram0_struct_set(0xFF00, vga_mode2_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode2_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode2_config_t, width_tiles, MAX_SCREEN_COL);
    xram0_struct_set(0xFF00, vga_mode2_config_t, height_tiles, MAX_SCREEN_ROW);
    xram0_struct_set(0xFF00, vga_mode2_config_t, xram_data_ptr, BACKGROUND_DATA);
    xram0_struct_set(0xFF00, vga_mode2_config_t, xram_palette_ptr, BACKGROUND_PALETTE);
    xram0_struct_set(0xFF00, vga_mode2_config_t, xram_tile_ptr, BACKGROUND_TILES);

    RIA.addr0 = BACKGROUND_DATA;
    RIA.step0 = 1;
    for (i = 0; i < BACKGROUND_TILES; i++)
    {
        RIA.rw0 = 45; // Fill map with tile 50, grass
    }


    xreg_vga_mode(2, 10, BACKGROUND_CONFIG,  sizeof(BACKGROUND_CONFIG), 2);
    RIA.addr0 = BACKGROUND_TILES;
    for (i = 0; i < GRASS_PATH_TILES_TOTAL_BYTES; i++)
    {
        RIA.rw0 = grass_path_tiles[i];
    }
}

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

    Input.up_pressed = false;
    Input.down_pressed = false;
    Input.left_pressed = false;
    Input.right_pressed = false;

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
        Input.up_pressed = true;
    if ((bits & 0x2) && !(bits & 0x1))
        Input.down_pressed = true;
    if ((bits & 0x4) && !(bits & 0x8))
        Input.left_pressed = true;
    if ((bits & 0x8) && !(bits & 0x4))
        Input.right_pressed = true;
    // byte 2 = BTN0: A(0), B(1), X(3), Y(4) for fire
    // if (RIA.rw1 & 0x1B)
    //     Input.shoot = true;

    if (!(keystates[0] & 1)) // any key pressed?
    {
        if (key(KEY_W) && !key(KEY_S))
            Input.up_pressed = true;
        if (key(KEY_S) && !key(KEY_W))
            Input.down_pressed = true;
        if (key(KEY_A) && !key(KEY_D))
            Input.left_pressed = true;
        if (key(KEY_D) && !key(KEY_A))
            Input.right_pressed = true;
    }
}
