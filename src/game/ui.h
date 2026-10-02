#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../xram.h"
#include "../game/game.h"

typedef struct {
    bool message_on;
    uint8_t message_counter;
    bool game_finshed;
} ui_t;

void ui_init(void);
void ui_show_message(char *message);
void ui_show_dialogue(char *dialogue);
void ui_clear_dialogue(void);
void ui_prepare_pause(void);
void ui_clear_pause(void);
void ui_prepare_skipped_frames(game_t *game);
void ui_prepare_draw(void);
void ui_draw(void);

#endif // UI_H