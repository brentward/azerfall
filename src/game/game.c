#include "game.h"
#include "player.h"
#include "../input/input.h"

static Player player;

void game_update(void)
{
    input_update();
    player_update(&player);
}

void draw(void)
{
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, x_pos_px, PLAYER_SCREEN_X - player.entity.world_x);
    xram0_struct_set(BACKGROUND_CONFIG, vga_mode2_config_t, y_pos_px, PLAYER_SCREEN_Y - player.entity.world_y);

    player_draw(&player);
}

// void animation_update(void)
// {
//     player_animation_update(&player);
// }

void game_init(void)
{
    player_init(&player);
    background_init();
    player_graphics_init();
    input_init();
}

void background_init(void)
{
    int i;
    xreg_vga_canvas(2);
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

    xreg_vga_mode(2, 10, BACKGROUND_CONFIG, 2);
}
