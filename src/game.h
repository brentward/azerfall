#ifndef GAME_H
#define GAME_H

#include <rp6502.h>
#include <stdio.h>
#include <stdbool.h>
#include "grass_path.h"
#include "player_sprites.h"

// Screen Settings

#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       180

#define TILE_SIZE            16
#define HALF_TILE_SIZE        8

#define MAX_SCREEN_COL       20
#define MAX_SCREEN_ROW       12  

#define BACKGROUND_TILES 0x1000
#define BACKGROUND_CONFIG 0xFF00
#define BACKGROUND_DATA  0x0000
#define BACKGROUND_PALETTE 0xFFFF
#define PLAYER_SPRITES (BACKGROUND_TILES + GRASS_PATH_TILES_TOTAL_BYTES)
#define PLAYER_SPRITES_CONFIG (BACKGROUND_CONFIG + sizeof(vga_mode2_config_t))
#define PLAYER_PALETTE 0xFFFF

void update_game(void);
void update_animation(void);
void draw(void);
void init_background(void);
void init_player_sprites(void);
void init_game(void);
void init_input(void);
void update_input(void);


#endif