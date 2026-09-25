#include "ui.h"

#include "../game/player.h"

Ui ui;

void ui_init(void)
{
    uint16_t i;

    xram0_struct_set(XRAM_UI_CONFIG, vga_mode1_config_t, x_wrap, false);
    xram0_struct_set(XRAM_UI_CONFIG, vga_mode1_config_t, y_wrap, false);
    xram0_struct_set(XRAM_UI_CONFIG, vga_mode1_config_t, x_pos_px, PLAYER_SCREEN_X - 50 );
    xram0_struct_set(XRAM_UI_CONFIG, vga_mode1_config_t, y_pos_px, PLAYER_SCREEN_Y + 50);
    xram0_struct_set(XRAM_UI_CONFIG, vga_mode1_config_t, width_chars, UI_MESSAGE_WIDTH_CHAR);
    xram0_struct_set(XRAM_UI_CONFIG, vga_mode1_config_t, height_chars, UI_MESSAGE_HEIGHT_CHAR);
    xram0_struct_set(XRAM_UI_CONFIG, vga_mode1_config_t, xram_data_ptr, XRAM_UI_MESSAGE);
    xram0_struct_set(XRAM_UI_CONFIG, vga_mode1_config_t, xram_palette_ptr, XRAM_DEFAULT_PALETTE);
    xram0_struct_set(XRAM_UI_CONFIG, vga_mode1_config_t, xram_font_ptr, XRAM_DEFAULT_FONT);

    RIA.addr0 = XRAM_UI_MESSAGE;
    RIA.step0 = 1;
    for (i = 0; i < UI_MESSAGE_SIZE; i++)
    {
        RIA.rw0 = 0; // data
        RIA.rw0 = 0; // fgbg color index
    }
    xreg_vga_mode1(MODE1_4BPP, XRAM_UI_CONFIG, VGA_PLANE_UI);
}

void ui_show_message(char *message)
{
    uint8_t i;
    char c;

    RIA.addr0 = XRAM_UI_MESSAGE;
    RIA.step0 = 1;
    for (i = 0; i < strlen(message); i++)
    {
        c = message[i];
        RIA.rw0 = c;
        RIA.rw0 = MODE1_BG_FG(ANSI_BLUE, ANSI_BRIGHT_YELLOW);
    }
    // strncpy(ui.message, message, UI_MESSAGE_SIZE - 1);
    // ui.message[UI_MESSAGE_SIZE - 1] = '\0';

    ui.message_on = true;
    ui.message_counter = 1;
}

void ui_prepare_pause(void)
{
    uint8_t i;
    char c;
    char *message = "PAUSE";

    RIA.addr0 = XRAM_UI_MESSAGE;
    RIA.step0 = 1;
    for (i = 0; i < strlen(message); i++)
    {
        c = message[i];
        RIA.rw0 = c;
        RIA.rw0 = MODE1_BG_FG(ANSI_BLUE, ANSI_BRIGHT_YELLOW);
    }
}

void ui_clear_pause(void)
{
    uint8_t i;

    RIA.addr0 = XRAM_UI_MESSAGE;
    RIA.step0 = 1;
    for (i = 0; i < UI_MESSAGE_SIZE; i++)
    {
        RIA.rw0 = 0; // data
        RIA.rw0 = 0; // fgbg color index
    }

}

void ui_prepare_draw(void)
{
    uint8_t i;

    if (ui.message_on && ui.message_counter == 0)
    {
        ui.message_on = false;
        RIA.addr0 = XRAM_UI_MESSAGE;
        RIA.step0 = 1;
        for (i = 0; i < UI_MESSAGE_SIZE; i++)
        {
            RIA.rw0 = 0; // data
            RIA.rw0 = 0; // fgbg color index
        }
    }
}

void ui_draw(void)
{
    if (ui.message_on)
    {
        if (++ui.message_counter > 121)
            ui.message_counter = 0;
    }
}
