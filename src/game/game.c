#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rp6502.h>

#include "player.h"
#include "../input/input.h"
#include "../../generated/world01_tiles.h"


static Player player;
static Game game;
static void background_upload(void);
static void background_init(void);
static void background_draw(void);

// /* Startup diagnostic: this reads the RIA's XRAM, not the VGA's replica. */
// static void verify_background_bytes(const char *label, unsigned address,
//                                     const uint8_t *expected, unsigned length)
// {
//     unsigned i;
//     uint8_t actual;

//     RIA.addr1 = address;
//     RIA.step1 = 1;
//     for (i = 0; i < length; i++)
//     {
//         actual = RIA.rw1;
//         if (actual != expected[i])
//         {
//             printf("BG XRAM FAIL %s at $%04X: expected %02X, read %02X\n",
//                    label, address + i, (unsigned)expected[i], (unsigned)actual);
//             exit(EXIT_FAILURE);
//         }
//     }
// }

// static void verify_background_upload(void)
// {
//     vga_mode2_config_t expected;

//     memset(&expected, 0, sizeof expected);
//     expected.x_wrap = false;
//     expected.y_wrap = false;
//     expected.x_pos_px = PLAYER_SCREEN_X - player.entity.world_x;
//     expected.y_pos_px = PLAYER_SCREEN_Y - player.entity.world_y;
//     expected.width_tiles = WORLD01_TILES_MAP_WIDTH;
//     expected.height_tiles = WORLD01_TILES_MAP_HEIGHT;
//     expected.xram_data_ptr = BACKGROUND_DATA;
//     expected.xram_palette_ptr = BACKGROUND_PALETTE;
//     expected.xram_tile_ptr = BACKGROUND_TILES;

//     verify_background_bytes("config", BACKGROUND_CONFIG,
//                             (const uint8_t *)&expected, sizeof expected);
//     verify_background_bytes("map", BACKGROUND_DATA, world01_tiles_map,
//                             WORLD01_TILES_MAP_TOTAL_BYTES);
//     verify_background_bytes("tiles", BACKGROUND_TILES, world01_tiles,
//                             WORLD01_TILES_TOTAL_BYTES);
//     puts("BG XRAM OK: config, map, tiles (RIA readback)");
// }

void game_update(void)
{
    input_update();
    if (input_state.background_reload_pressed)
    {
        background_upload();
        // verify_background_upload();
        puts("BG re-upload complete (no VGA mode change)");
    }

    if (input_state.pause_pressed)
    {
        switch (game.state) {
        case GAME_STATE_PLAY:
            game.state = GAME_STATE_PAUSE;
            break;

        case GAME_STATE_PAUSE:
            game.state = GAME_STATE_PLAY;
            break;
        }
    }

    if (game.state == GAME_STATE_PLAY)
    {
        player_update(&player);
    }
}

void draw(void)
{
    background_draw();
    player_draw(&player);
}

void background_draw(void)
{
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, x_pos_px, PLAYER_SCREEN_X - player.entity.world_x);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, y_pos_px, PLAYER_SCREEN_Y - player.entity.world_y);
}

void game_init(void)
{
    memset(&game, 0, sizeof game);
    game.state = GAME_STATE_PLAY;
    player_init(&player);
    background_init();
    player_graphics_init();
    input_init();
    // /* Check after all uploads so later initialization overwrites are caught. */
    // verify_background_upload();
}

static void background_init(void)
{
    unsigned char frame;

    xreg_vga_canvas(2);
    // if (xreg_vga_canvas(2) < 0)
    // {
    //     perror("VGA canvas setup");
    //     exit(EXIT_FAILURE);
    // }

    /* Timing diagnostic: wait for the next VSYNC before the first upload. */
    frame = RIA.vsync;
    while (RIA.vsync == frame)
    {
    }
    background_upload();

    xreg_vga_mode(2, 10, BACKGROUND_CONFIG, 2);
    // if (xreg_vga_mode(2, 10, BACKGROUND_CONFIG, 2) < 0)
    // {
    //     perror("VGA background setup");
    //     exit(EXIT_FAILURE);
    // }
}

/* Shared with the R-key diagnostic; only writes XRAM, never VGA registers. */
static void background_upload(void)
{
    int i;

    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, x_wrap, false);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, y_wrap, false);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, x_pos_px, PLAYER_SCREEN_X - player.entity.world_x);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, y_pos_px, PLAYER_SCREEN_Y - player.entity.world_y);
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


    RIA.addr0 = BACKGROUND_TILES;
    for (i = 0; i < WORLD01_TILES_TOTAL_BYTES; i++)
    {
        RIA.rw0 = world01_tiles[i];
    }

}
