#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>

// Screen Settings
#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       180

#define TILE_SIZE            16
#define HALF_TILE_SIZE        8

#define MAX_SCREEN_COL       20
#define MAX_SCREEN_ROW       12  

// World Settings
#define WORLD_WIDTH_TILES  50
#define WORLD_HEIGHT_TILES 50

#define BYTES_PER_SPRITE 128

typedef enum {
    GAME_STATE_TITLE_SCREEN,
    GAME_STATE_PLAY,
    GAME_STATE_PAUSE,
    GAME_STATE_DIALOGUE,
    GAME_STATE_CHARACTER,
    GAME_STATE_OPTIONS,
    GAME_STATE_GAME_OVER,
    GAME_STATE_TRANSITION,
    GAME_STATE_TRADE,
    GAME_STATE_SLEEP,
    GAME_STATE_MAP,
    GAME_STATE_CUTSCENE
} game_state_t;

typedef struct {
    game_state_t state;
    uint16_t song_xram_ptr;
    uint16_t song_delay;
    uint16_t song_size;
    uint16_t song_loop_offset;
    uint16_t song_bytes_remaining; /* Zero when unloaded or stopped. */
    uint8_t background_animation_timer;
    bool srand_init;
    uint16_t skipped_frames;
} game_t;

void game_update(void);
void timed_update();
void audio_update(uint8_t elapsed_frames);
void game_init(void);
void game_state_set(game_state_t new_state);

#endif
