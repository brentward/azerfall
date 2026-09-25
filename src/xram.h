/* RP6502 XRAM Mapping */

#ifndef XRAM_H
#define XRAM_H

#include <rp6502.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../generated/world_tiles_worldmap_map.h"
#include "../generated/player_sprites.h"
#include "../generated/object_sprites.h"

/* Insert xram.h snippets from docs to build your system. */
/* https://picocomputer.github.io/sdk.html#xram-memory-map */
#define xreg_ria_opl(...) xreg(0, 1, 1, __VA_ARGS__)
#define xreg_vga_mode1(...) xreg(1, 0, 1, 1, __VA_ARGS__)
#define xreg_vga_mode2(...) xreg(1, 0, 1, 2, __VA_ARGS__)
#define xreg_vga_mode5(...) xreg(1, 0, 1, 5, __VA_ARGS__)

/* Separate planes keep UI fill programming from replacing the tile map. */
#define VGA_PLANE_WORLD 0
#define VGA_PLANE_SPRITES 1
#define VGA_PLANE_UI 2

#define CANVAS_CONSOLE 0
#define CANVAS_320X240 1
#define CANVAS_320X180 2
#define CANVAS_640X480 3
#define CANVAS_640X360 4

#define MODE1_1BPP 0x00
#define MODE1_4BPPR 0x01
#define MODE1_4BPP 0x02
#define MODE1_8BPP 0x03
#define MODE1_16BPP 0x04

#define MODE1_8X8 0x00
#define MODE1_8X16 0x08

#define MODE1_FG_BG(fg, bg) ((uint8_t)(((fg) << 4) | (bg)))
#define MODE1_BG_FG(bg, fg) ((uint8_t)(((bg) << 4) | (fg)))

#define UI_MESSAGE_WIDTH_CHAR 24
#define UI_MESSAGE_HEIGHT_CHAR 2
#define UI_MESSAGE_SIZE (UI_MESSAGE_WIDTH_CHAR * UI_MESSAGE_HEIGHT_CHAR + 1)


#define MODE2_1BPP 0x00
#define MODE2_2BPP 0x01
#define MODE2_4BPP 0x02
#define MODE2_8BPP 0x03

#define MODE2_8X8 0x00
#define MODE2_16X16 0x08

#define MODE2_X_TRIM(cols) ((cols) << 4)
#define MODE2_Y_TRIM(rows) ((rows) << 8)

#define MODE5_1BPP 0x00
#define MODE5_2BPP 0x01
#define MODE5_4BPP 0x02
#define MODE5_8BPP 0x03

#define MODE5_8X8 0x00
#define MODE5_16X16 0x08
#define MODE5_32X32 0x10
#define MODE5_64X64 0x18
#define MODE5_128X128 0x20
#define MODE5_256X256 0x28
#define MODE5_512X512 0x30

#define SPRITE_LIMIT   32U

#define COLOR_FROM_RGB8(r, g, b) \
    ((((unsigned)(b) >> 3) << 11) | (((unsigned)(g) >> 3) << 6) | ((unsigned)(r) >> 3))
#define COLOR_FROM_RGB5(r, g, b) \
    (((unsigned)(b) << 11) | ((unsigned)(g) << 6) | (unsigned)(r))
#define COLOR_ALPHA_MASK (1u << 5)

/* Built-in ANSI palette indices */
#define ANSI_BLACK           0x00
#define ANSI_RED             0x01
#define ANSI_GREEN           0x02
#define ANSI_YELLOW          0x03
#define ANSI_BLUE            0x04
#define ANSI_MAGENTA         0x05
#define ANSI_CYAN            0x06
#define ANSI_WHITE           0x07
#define ANSI_BRIGHT_BLACK    0x08  /* Dark gray */
#define ANSI_BRIGHT_RED      0x09
#define ANSI_BRIGHT_GREEN    0x0A
#define ANSI_BRIGHT_YELLOW   0x0B
#define ANSI_BRIGHT_BLUE     0x0C
#define ANSI_BRIGHT_MAGENTA  0x0D
#define ANSI_BRIGHT_CYAN     0x0E
#define ANSI_BRIGHT_WHITE    0x0F

#define SONG_DATA_MAX_BYTES 0x8000U

#define GAMEPAD_PLAYERS 4

#define GAMEPAD_DPAD_UP 0x01
#define GAMEPAD_DPAD_DOWN 0x02
#define GAMEPAD_DPAD_LEFT 0x04
#define GAMEPAD_DPAD_RIGHT 0x08

#define GAMEPAD_FEAT_TYPE_MASK 0x30
#define GAMEPAD_TYPE_UNKNOWN 0x00
#define GAMEPAD_TYPE_WESTERN 0x10
#define GAMEPAD_TYPE_EASTERN 0x20
#define GAMEPAD_TYPE_PLAYSTATION 0x30
#define GAMEPAD_FEAT_STICKS 0x40
#define GAMEPAD_FEAT_CONNECTED 0x80

#define GAMEPAD_LSTICK_UP 0x01
#define GAMEPAD_LSTICK_DOWN 0x02
#define GAMEPAD_LSTICK_LEFT 0x04
#define GAMEPAD_LSTICK_RIGHT 0x08
#define GAMEPAD_RSTICK_UP 0x10
#define GAMEPAD_RSTICK_DOWN 0x20
#define GAMEPAD_RSTICK_LEFT 0x40
#define GAMEPAD_RSTICK_RIGHT 0x80

#define GAMEPAD_BTN0_A 0x01
#define GAMEPAD_BTN0_B 0x02
#define GAMEPAD_BTN0_C 0x04
#define GAMEPAD_BTN0_X 0x08
#define GAMEPAD_BTN0_Y 0x10
#define GAMEPAD_BTN0_Z 0x20
#define GAMEPAD_BTN0_L1 0x40
#define GAMEPAD_BTN0_R1 0x80

#define GAMEPAD_BTN1_L2 0x01
#define GAMEPAD_BTN1_R2 0x02
#define GAMEPAD_BTN1_SELECT 0x04
#define GAMEPAD_BTN1_START 0x08
#define GAMEPAD_BTN1_HOME 0x10
#define GAMEPAD_BTN1_L3 0x20
#define GAMEPAD_BTN1_R3 0x40

#define OPL_REGISTERS_SIZE 256U

typedef struct
{
    uint8_t reg[OPL_REGISTERS_SIZE];
} opl_t;

typedef struct
{
    bool x_wrap;
    bool y_wrap;
    int16_t x_pos_px;
    int16_t y_pos_px;
    int16_t width_chars;
    int16_t height_chars;
    uint16_t xram_data_ptr;
    uint16_t xram_palette_ptr;
    uint16_t xram_font_ptr;
} mode1_config_t;

typedef struct
{
    uint8_t glyph_code;
    uint8_t fg_bg_index;
} mode1_4bppr_data_t;

typedef struct
{
    uint8_t glyph_code;
    uint8_t bg_fg_index;
} mode1_4bpp_data_t;

typedef struct
{
    uint8_t glyph_code;
    uint8_t fg_index;
    uint8_t bg_index;
} mode1_8bpp_data_t;

typedef struct
{
    uint8_t glyph_code;
    uint8_t attributes;
    uint16_t fg_color;
    uint16_t bg_color;
} mode1_16bpp_data_t;

#define MODE2_TILE(bpp, size)                 \
    struct                                    \
    {                                         \
        struct                                \
        {                                     \
            uint8_t cols[(size) * (bpp) / 8]; \
        } rows[size];                         \
    }


typedef struct
{
    bool x_wrap;
    bool y_wrap;
    int16_t x_pos_px;
    int16_t y_pos_px;
    int16_t width_tiles;
    int16_t height_tiles;
    uint16_t xram_data_ptr;
    uint16_t xram_palette_ptr;
    uint16_t xram_tile_ptr;
} mode2_config_t;


#define MODE5_IMAGE(bpp, size)                \
    struct                                    \
    {                                         \
        struct                                \
        {                                     \
            uint8_t cols[(size) * (bpp) / 8]; \
        } rows[size];                         \
    }

typedef struct
{
    int16_t x_pos_px;
    int16_t y_pos_px;
    uint16_t xram_sprite_ptr;
    uint16_t palette_ptr;
} mode5_sprite_t;

typedef struct
{
    uint8_t keys[32];
} keyboard_t;

typedef struct
{
    struct
    {
        uint8_t dpad;
        uint8_t sticks;
        uint8_t btn0;
        uint8_t btn1;
        int8_t lx;
        int8_t ly;
        int8_t rx;
        int8_t ry;
        uint8_t l2;
        uint8_t r2;
    } player[GAMEPAD_PLAYERS];
} gamepad_t;

/* Insert XRAM structs from those snippets here along with */
/* your own custom usage to define your XRAM memory map. */
typedef struct
{
    opl_t opl;
    uint8_t world_map[WORLD_TILES_WORLDMAP_MAP_TOTAL_BYTES];
    MODE2_TILE(WORLD_TILES_BPP, WORLD_TILES_TILE_WIDTH)
        world_tiles[WORLD_TILES_TILE_COUNT];
    MODE5_IMAGE(PLAYER_SPRITES_BPP, PLAYER_SPRITES_WIDTH) player_images[PLAYER_SPRITES_COUNT];
    MODE5_IMAGE(OBJECT_SPRITES_BPP, OBJECT_SPRITES_WIDTH) object_images[OBJECT_SPRITES_COUNT];
    uint8_t song_data[SONG_DATA_MAX_BYTES];
    mode1_4bpp_data_t ui_message[UI_MESSAGE_SIZE];
    uint16_t world_palette[1 << WORLD_TILES_BPP];
    mode5_sprite_t sprite_configs[SPRITE_LIMIT];
    mode1_config_t ui_config;
    mode2_config_t bg_config;
    keyboard_t keyboard;
    gamepad_t gamepad;
} xram_layout_t;


#define XRAM_TOTAL 0x0000U
#define XRAM_USED sizeof(xram_layout_t)
#define XRAM_FREE (XRAM_TOTAL - XRAM_USED)

/* Define constants to be your XRAM addresses. */
#define XRAM_OPL offsetof(xram_layout_t, opl)
#define XRAM_WORLD_MAP offsetof(xram_layout_t, world_map)
#define XRAM_WORLD_TILES offsetof(xram_layout_t, world_tiles)
#define XRAM_PLAYER_IMAGES offsetof(xram_layout_t, player_images)
#define XRAM_OBJECT_IMAGES offsetof(xram_layout_t, object_images)
#define XRAM_SONG_DATA offsetof(xram_layout_t, song_data)
#define XRAM_WORLD_PALETTE offsetof(xram_layout_t, world_palette)
#define XRAM_UI_MESSAGE offsetof(xram_layout_t, ui_message)
#define XRAM_SPRITE_CONFIGS offsetof(xram_layout_t, sprite_configs)
#define XRAM_SPRITE_CONFIG(slot) \
    (XRAM_SPRITE_CONFIGS + (slot) * sizeof(vga_mode5_sprite_t))
#define XRAM_UI_CONFIG offsetof(xram_layout_t, ui_config)
#define XRAM_BG_CONFIG offsetof(xram_layout_t, bg_config)
#define XRAM_KEYBOARD offsetof(xram_layout_t, keyboard)
#define XRAM_GAMEPAD offsetof(xram_layout_t, gamepad)

#define XRAM_DEFAULT_FONT 0xFFFFU
#define XRAM_DEFAULT_PALETTE 0xFFFFU

#endif
