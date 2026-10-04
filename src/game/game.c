#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rp6502.h>

#include "../xram.h"
#include "../entity/player.h"
#include "ui.h"
#include "../object/object.h"
#include "../input/input.h"
#include "../audio/music.h"
#include "../audio/sound.h"
#include "../entity/npc.h"


static player_t player;
static game_t game;
static object_t objects[OBJECT_COUNT];
static npc_t npcs[NPC_COUNT];

// static vga_mode5_sprite_t sprite_configs[8];

// static void background_upload(void);
static void background_copy_tile(uint8_t destination, uint8_t source);
static void background_init(void);
static void background_draw(void);
static void set_objects(void);
static void set_npcs(void);

void game_init(void)
{
    unsigned char frame;

    memset(&game, 0, sizeof game);
    game.state =  GAME_STATE_TITLE_SCREEN;
    /* Reset scanline programming before uploading this run's graphics. */
    game_set_canvas();
    frame = RIA.vsync;
    while (RIA.vsync == frame)
    {
    }
    player_init(&player);
    object_sprite_init();
    npc_sprite_init();
    background_init();
    set_objects();
    set_npcs();
    player_graphics_init();
    ui_init();
    input_init();
    sound_init();
    music_init(&game, "ROM:Z3LIGHTW.BIN", true);
    ui_title_set_vga_mode();
    ui_message_set_vga_mode();
    /* Z3LIGHTW: sequence 03, row 00 = frame 424, BIN byte 4116.
     * Recalculate after re-exporting the song (tools/check_music_loop.py). */
    if (!music_set_loop_offset(&game, 4116U))
        puts("Invalid music loop point");
#ifdef MUSIC_AUDITION
    /* Keep the final five seconds, then hear the jump to sequence 03. */
    music_skip_to_frame(&game, 4358U);
#else
    /* Hold the song at the beginning until the title screen starts gameplay. */
    music_pause();
#endif
    // /* Check after all uploads so later initialization overwrites are caught. */
    // verify_background_upload();
}

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
    uint8_t i;
    object_t *object;
    npc_t *npc;
    input_update();
    if (input_state.srand_seeded && !game.srand_init)
    {
        srand(input_state.counter);
        // srand(1234);
        game.srand_init = true;
    }
    if (input_state.background_reload_pressed)
    {
        background_init();
        // verify_background_upload();
        puts("BG re-upload complete (no VGA mode change)");
    }

    if (input_state.pause_pressed)
    {
        switch (game.state) {
        case GAME_STATE_PLAY:
            game.state = GAME_STATE_PAUSE;
            music_pause();
            ui_prepare_pause();
            break;

        case GAME_STATE_PAUSE:
            game.state = GAME_STATE_PLAY;
            music_resume();
            ui_clear_pause();
            break;
        }
    }

    if (input_state.interact_pressed)
    {
        switch (game.state) {
        case GAME_STATE_DIALOGUE:
            game.state = GAME_STATE_PLAY;
            ui_clear_dialogue();
            break;
        }
    }

    if (game.state == GAME_STATE_TITLE_SCREEN)
    {
        ui_update_titlescreen();
        if (game.state == GAME_STATE_TITLE_SCREEN)
            ui_prepare_titlescreen();
        return;
    }


    if (game.state == GAME_STATE_PLAY)
    {
        player_update(&player, objects, npcs);
        for (i = 0, object = objects; i < OBJECT_COUNT; i++, object++)
        {
            object_prepare_draw(object, &player);
        }
        for (i = 0, npc = npcs; i < NPC_COUNT; i++, npc++)
        {
            entity_t *entity = &npc->entity;

            if (entity->hitbox.width == 0 || entity->hitbox.height == 0)
                continue;
            entity_update(entity, &player, objects, npcs);
            entity_prepare_draw(entity, &player);
        }
        ui_prepare_skipped_frames(&game);
        ui_prepare_draw();
    }
}

void game_state_set(game_state_t new_state )
{
    game.state = new_state;
}

void timed_update()
{
    uint8_t i;
    object_t *object;
    npc_t *npc;
    background_draw();
    ui_draw();
    player_draw(&player);
    for (i = 0, object = objects; i < OBJECT_COUNT; i++, object++)
    {
        object_draw(object, i + 1);
    }
    for (i = 0, npc = npcs; i < NPC_COUNT; i++, npc++)
    {
        if (npc->entity.hitbox.width == 0 || npc->entity.hitbox.height == 0)
            continue;
        entity_draw(&npc->entity, i + 8);
    }

    if (++game.background_animation_timer == 60)
    {
        background_copy_tile(2, 38);
        background_copy_tile(19, 39);
    }
    if (game.background_animation_timer == 120)
    {
        background_copy_tile(2, 2);
        background_copy_tile(19, 19);
        game.background_animation_timer = 0;
    }

}

void audio_update(uint8_t elapsed_frames)
{
    /* BIN delays and SFX envelopes use the 60 Hz clock, not render count. */
    if (elapsed_frames > 1)
    {
        game.skipped_frames += elapsed_frames - 1;
    }
    while (elapsed_frames != 0)
    {
        music_update(&game);
        sound_update();
        --elapsed_frames;
    }
}

// TODO animate grass by cycling tile 2 with 38 and water 19 with 39
static void background_draw(void)
{
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, x_pos_px, PLAYER_SCREEN_X - player.entity.world_x);
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, y_pos_px, PLAYER_SCREEN_Y - player.entity.world_y);
}

// static void background_mode_set(void)
// {
//     unsigned char frame;

//     // xreg_vga_canvas(2);
//     if (xreg_vga_canvas(CANVAS_320X180) < 0)
//     {
//         perror("VGA canvas setup");
//         exit(EXIT_FAILURE);
//     }

//     /* Timing diagnostic: wait for the next VSYNC before the first upload. */
//     frame = RIA.vsync;
//     while (RIA.vsync == frame)
//     {
//     }
//     // background_upload();

//     if (xreg_vga_mode2(MODE2_4BPP | MODE2_16X16, XRAM_BG_CONFIG, VGA_PLANE_WORLD) < 0)
//     {
//         perror("VGA background setup");
//         exit(EXIT_FAILURE);
//     }
// }

/* Shared with the R-key diagnostic; only writes XRAM, never VGA registers. */
static void background_init(void)
{
    uint16_t i;
    const uint8_t *pixels;
    const uint16_t *palette;

    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, x_wrap, false);
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, y_wrap, false);
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, x_pos_px, PLAYER_SCREEN_X - player.entity.world_x);
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, y_pos_px, PLAYER_SCREEN_Y - player.entity.world_y);
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, width_tiles, WORLD_TILES_WORLDMAP_MAP_WIDTH);
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, height_tiles, WORLD_TILES_WORLDMAP_MAP_HEIGHT);
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, xram_data_ptr, XRAM_WORLD_MAP);
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, xram_palette_ptr, XRAM_WORLD_PALETTE);
    xram0_struct_set(XRAM_BG_CONFIG, vga_mode2_config_t, xram_tile_ptr, XRAM_WORLD_TILES);

    RIA.addr0 = XRAM_WORLD_MAP;
    RIA.step0 = 1;
    for (i = 0, pixels = world_tiles_worldmap_map; i < WORLD_TILES_WORLDMAP_MAP_TOTAL_BYTES; i++, pixels++)
    {
        RIA.rw0 = *pixels;
    }


    RIA.addr0 = XRAM_WORLD_TILES;
    for (i = 0, pixels = world_tiles; i < WORLD_TILES_TOTAL_BYTES; i++, pixels++)
    {
        RIA.rw0 = *pixels;
    }

    RIA.addr0 = XRAM_WORLD_PALETTE;
    for (i = 0, palette = world_tiles_palette; i < WORLD_TILES_PALETTE_COUNT; i++, palette++)
    {
        RIA.rw0 = (uint8_t)*palette;
        RIA.rw0 = (uint8_t)(*palette >> 8);
    }

}

void game_set_canvas(void)
{
    if (xreg_vga_canvas(CANVAS_320X180) < 0)
    {
        perror("VGA canvas setup");
        exit(EXIT_FAILURE);
    }
}

void background_set_vga_mode(void)
{
    /* Timing diagnostic: wait for the next VSYNC before the first upload. */
    // unsigned char frame = RIA.vsync;
    // while (RIA.vsync == frame)
    // {
    // }
    // background_init();

    if (xreg_vga_mode2(MODE2_4BPP | MODE2_16X16, XRAM_BG_CONFIG, VGA_PLANE_WORLD) < 0)
    {
        perror("VGA background setup");
        exit(EXIT_FAILURE);
    }
}

void sprite_set_vga_mode(void)
{
    xreg_vga_mode5(MODE5_4BPP | MODE5_16X16, XRAM_SPRITE_CONFIG(PLAYER_SPRITE_SLOT), TOTAL_SPRITE_COUNT, VGA_PLANE_SPRITES);
}

static void background_copy_tile(uint8_t destination, uint8_t source)
{
    uint8_t i;
    const uint8_t *pixels;

    pixels = world_tiles + (uint16_t)source * WORLD_TILES_BYTES_PER_TILE;
    RIA.addr0 = XRAM_WORLD_TILES
        + (uint16_t)destination * WORLD_TILES_BYTES_PER_TILE;
    RIA.step0 = 1;

    for (i = 0; i < WORLD_TILES_BYTES_PER_TILE; ++i, ++pixels)
        RIA.rw0 = *pixels;
}

static void set_objects(void)
{
    init_key(&objects[0], &player, 1, 368, 112);
    init_key(&objects[1], &player, 2, 368, 640);
    init_key(&objects[2], &player, 3, 608, 128);
    init_door(&objects[3], &player, 4, 192, 192);
    init_door(&objects[4], &player, 5, 160, 448);
    init_door(&objects[5], &player, 6, 208, 368);
    init_chest(&objects[6], &player, 7, 192, 144);
}


static void set_npcs(void)
{
    npc_oldman_init(&npcs[0].entity, &player, 8, 336, 336);
    npc_oldman_set_dialogue(
        &npcs[0],
        "Hello there!",
        "I am the old man,",
        "I have lived here\nfor many years.",
        "It is dangerous\nto go alone!\nTake this."
    );
    
}

void game_start(void)
{
    game.state = GAME_STATE_PLAY;
    music_resume();
    game_set_canvas();
    background_set_vga_mode();
    sprite_set_vga_mode();
    ui_upper_set_vga_mode();
    ui_pause_set_vga_mode();
    ui_message_set_vga_mode();
    ui_lower_set_vga_mode();
}
