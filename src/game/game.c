#include "game.h"
#include "player.h"
#include "../input/input.h"
// #include "../input/usb_hid_keys.h"

// InputState input_state = {0};

// uint8_t keystates[KEYBOARD_BYTES] = {0};


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
    xreg_vga_canvas(2);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, x_wrap, false);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, y_wrap, false);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, x_pos_px, player.entity.world_x - PLAYER_SCREEN_X);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, y_pos_px, player.entity.world_y - PLAYER_SCREEN_Y);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, width_tiles, WORLD01_TILES_MAP_WIDTH);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, height_tiles, WORLD01_TILES_MAP_HEIGHT);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, xram_data_ptr, BACKGROUND_DATA);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, xram_palette_ptr, BACKGROUND_PALETTE);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, xram_tile_ptr, BACKGROUND_TILES);

    RIA.addr0 = BACKGROUND_DATA;
    RIA.step0 = 1;
    for (i = 0; i < WORLD01_TILES_MAP_TOTAL_BYTES; i++)
    {
        RIA.rw0 = world01_tiles_map[i];
    }


    xreg_vga_mode(2, 10, BACKGROUND_CONFIG,  sizeof(BACKGROUND_CONFIG), 2);
    RIA.addr0 = BACKGROUND_TILES;
    for (i = 0; i < WORLD01_TILES_TOTAL_BYTES; i++)
    {
        RIA.rw0 = world01_tiles[i];
    }
}

// void input_init(void)
// {
//     int i;

//     // Start with no input before enabling live updates from the RIA.
//     RIA.addr0 = KEYBOARD_INPUT;
//     RIA.step0 = 1;
//     for (i = 0; i < KEYBOARD_BYTES; i++)
//         RIA.rw0 = (i == 0) ? 1 : 0;

//     RIA.addr0 = GAMEPAD_INPUT;
//     for (i = 0; i < 40; i++)
//         RIA.rw0 = 0;

//     xreg_ria_keyboard(KEYBOARD_INPUT);
//     xreg_ria_gamepad(GAMEPAD_INPUT);
// }


// #define GAMEPAD_SIZE 10         // bytes per gamepad in XRAM
// #define GAMEPAD_CONNECTED 0x80  // byte 0 bit 7
// #define GAMEPAD_BTN_START 0x08  // byte 3 bit 3
// #define GAMEPAD_BTN_SELECT 0x04 // byte 3 bit 2

// void input_update(void)
// {
//     int bits;
//     unsigned char dpad;
//     unsigned char sticks;
//     int i;

//     input_state.up_pressed = false;
//     input_state.down_pressed = false;
//     input_state.left_pressed = false;
//     input_state.right_pressed = false;

//     RIA.addr1 = KEYBOARD_INPUT;
//     RIA.step1 = 1;
//     for (i = 0; i < KEYBOARD_BYTES; i++)
//         keystates[i] = RIA.rw1;

//     RIA.addr1 = GAMEPAD_INPUT;
//     // byte 0 = dpad, byte 1 = sticks: merge for direction
//     dpad = RIA.rw1;
//     sticks = RIA.rw1;
//     bits = (dpad & GAMEPAD_CONNECTED) ? ((dpad | sticks) & 0x0F) : 0;
//     if ((bits & 0x1) && !(bits & 0x2))
//         input_state.up_pressed = true;
//     if ((bits & 0x2) && !(bits & 0x1))
//         input_state.down_pressed = true;
//     if ((bits & 0x4) && !(bits & 0x8))
//         input_state.left_pressed = true;
//     if ((bits & 0x8) && !(bits & 0x4))
//         input_state.right_pressed = true;
//     // byte 2 = BTN0: A(0), B(1), X(3), Y(4) for fire
//     // if (RIA.rw1 & 0x1B)
//     //     Input.shoot = true;

//     if (!(keystates[0] & 1)) // any key pressed?
//     {
//         if (key(KEY_W) && !key(KEY_S))
//             input_state.up_pressed = true;
//         if (key(KEY_S) && !key(KEY_W))
//             input_state.down_pressed = true;
//         if (key(KEY_A) && !key(KEY_D))
//             input_state.left_pressed = true;
//         if (key(KEY_D) && !key(KEY_A))
//             input_state.right_pressed = true;
//     }
// }
