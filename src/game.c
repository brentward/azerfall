#include "game.h"

void update_game(void)
{
    // Update game logic here
}

void update_animation(void)
{
    // Update animation logic here
}

void draw(void)
{
    // Draw game elements here
}

void init_game(void)
{
    init_background();
    init_player_sprites();
    init_input();
}

void init_background(void)
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

    xram0_struct_set(PLAYER_SPRITES_CONFIG, vga_mode5_sprite_t, x_pos_px, 160);
    xram0_struct_set(PLAYER_SPRITES_CONFIG, vga_mode5_sprite_t, y_pos_px, 90);
    xram0_struct_set(PLAYER_SPRITES_CONFIG, vga_mode5_sprite_t, xram_sprite_ptr, PLAYER_SPRITES);
    xram0_struct_set(PLAYER_SPRITES_CONFIG, vga_mode5_sprite_t, palette_ptr, PLAYER_PALETTE);

    RIA.addr0 = BACKGROUND_DATA;
    RIA.step0 = 1;
    for (i = 0; i < BACKGROUND_TILES; i++)
    {
        RIA.rw0 = 45; // Fill map with tile 50, grass
    }


    xreg_vga_mode(2, 10, BACKGROUND_CONFIG);
    RIA.addr0 = BACKGROUND_TILES;
    for (i = 0; i < GRASS_PATH_TILES_TOTAL_BYTES; i++)
    {
        RIA.rw0 = grass_path_tiles[i];
    }
}

void init_player_sprites(void)
{
    int i;
    RIA.addr0 = PLAYER_SPRITES;
    for (i = 0; i < PLAYER_SPRITES_TOTAL_BYTES; i++)
    {
        RIA.rw0 = player_sprites[i];
    }
}

void init_input(void)
{
    // Initialize input handling here
}

void update_input(void)
{
    // Update input handling here
}
