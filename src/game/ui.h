#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../xram.h"

typedef struct Ui {
    bool message_on;
    // char message[UI_MESSAGE_SIZE];
    uint8_t message_counter;
    bool game_finshed;
} Ui;

void ui_init(void);
void ui_show_message(char *message);
void ui_prepare_pause(void);
void ui_clear_pause(void);
void ui_prepare_draw(void);
void ui_draw(void);

#endif // UI_H