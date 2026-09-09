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

#define BYTES_PER_SPRITE 128

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

extern InputState Input;


void game_update(void);
void animation_update(void);
void draw(void);
void background_init(void);
void game_init(void);
void input_init(void);
void input_update(void);


#endif
