#ifndef GAME_H
#define GAME_H

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
} GameState;

typedef struct {
    GameState state;
} Game;

void game_update(void);
void draw(void);
void game_init(void);

#endif
