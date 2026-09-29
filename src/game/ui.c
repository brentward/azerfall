#include "ui.h"

#include "../game/player.h"

ui_t ui;

void ui_init(void)
{
    uint16_t i;

    char *message;
    char c;


    xram0_struct_set(XRAM_UI_UPPER_CONFIG, vga_mode1_config_t, x_wrap, false);
    xram0_struct_set(XRAM_UI_UPPER_CONFIG, vga_mode1_config_t, y_wrap, false);
    xram0_struct_set(XRAM_UI_UPPER_CONFIG, vga_mode1_config_t, x_pos_px, UI_UPPER_X_POS_PX);
    xram0_struct_set(XRAM_UI_UPPER_CONFIG, vga_mode1_config_t, y_pos_px, UI_UPPER_Y_POS_PX);
    xram0_struct_set(XRAM_UI_UPPER_CONFIG, vga_mode1_config_t, width_chars, UI_UPPER_WIDTH_CHAR);
    xram0_struct_set(XRAM_UI_UPPER_CONFIG, vga_mode1_config_t, height_chars, UI_UPPER_HEIGHT_CHAR);
    xram0_struct_set(XRAM_UI_UPPER_CONFIG, vga_mode1_config_t, xram_data_ptr, XRAM_UI_UPPER);
    xram0_struct_set(XRAM_UI_UPPER_CONFIG, vga_mode1_config_t, xram_palette_ptr, XRAM_DEFAULT_PALETTE);
    xram0_struct_set(XRAM_UI_UPPER_CONFIG, vga_mode1_config_t, xram_font_ptr, XRAM_DEFAULT_FONT);

    xram0_struct_set(XRAM_UI_PAUSE_CONFIG, vga_mode1_config_t, x_wrap, false);
    xram0_struct_set(XRAM_UI_PAUSE_CONFIG, vga_mode1_config_t, y_wrap, false);
    xram0_struct_set(XRAM_UI_PAUSE_CONFIG, vga_mode1_config_t, x_pos_px, UI_PAUSE_X_POS_PX);
    xram0_struct_set(XRAM_UI_PAUSE_CONFIG, vga_mode1_config_t, y_pos_px, UI_PAUSE_Y_POS_PX);
    xram0_struct_set(XRAM_UI_PAUSE_CONFIG, vga_mode1_config_t, width_chars, UI_PAUSE_WIDTH_CHAR);
    xram0_struct_set(XRAM_UI_PAUSE_CONFIG, vga_mode1_config_t, height_chars, UI_PAUSE_HEIGHT_CHAR);
    xram0_struct_set(XRAM_UI_PAUSE_CONFIG, vga_mode1_config_t, xram_data_ptr, XRAM_UI_PAUSE);
    xram0_struct_set(XRAM_UI_PAUSE_CONFIG, vga_mode1_config_t, xram_palette_ptr, XRAM_DEFAULT_PALETTE);
    xram0_struct_set(XRAM_UI_PAUSE_CONFIG, vga_mode1_config_t, xram_font_ptr, XRAM_DEFAULT_FONT);

    xram0_struct_set(XRAM_UI_MESSAGE_CONFIG, vga_mode1_config_t, x_wrap, false);
    xram0_struct_set(XRAM_UI_MESSAGE_CONFIG, vga_mode1_config_t, y_wrap, false);
    xram0_struct_set(XRAM_UI_MESSAGE_CONFIG, vga_mode1_config_t, x_pos_px, UI_MESSAGE_X_POS_PX);
    xram0_struct_set(XRAM_UI_MESSAGE_CONFIG, vga_mode1_config_t, y_pos_px, UI_MESSAGE_Y_POS_PX);
    xram0_struct_set(XRAM_UI_MESSAGE_CONFIG, vga_mode1_config_t, width_chars, UI_MESSAGE_WIDTH_CHAR);
    xram0_struct_set(XRAM_UI_MESSAGE_CONFIG, vga_mode1_config_t, height_chars, UI_MESSAGE_HEIGHT_CHAR);
    xram0_struct_set(XRAM_UI_MESSAGE_CONFIG, vga_mode1_config_t, xram_data_ptr, XRAM_UI_MESSAGE);
    xram0_struct_set(XRAM_UI_MESSAGE_CONFIG, vga_mode1_config_t, xram_palette_ptr, XRAM_DEFAULT_PALETTE);
    xram0_struct_set(XRAM_UI_MESSAGE_CONFIG, vga_mode1_config_t, xram_font_ptr, XRAM_DEFAULT_FONT);

    xram0_struct_set(XRAM_UI_LOWER_CONFIG, vga_mode1_config_t, x_wrap, false);
    xram0_struct_set(XRAM_UI_LOWER_CONFIG, vga_mode1_config_t, y_wrap, false);
    xram0_struct_set(XRAM_UI_LOWER_CONFIG, vga_mode1_config_t, x_pos_px, UI_LOWER_X_POS_PX);
    xram0_struct_set(XRAM_UI_LOWER_CONFIG, vga_mode1_config_t, y_pos_px, UI_LOWER_Y_POS_PX);
    xram0_struct_set(XRAM_UI_LOWER_CONFIG, vga_mode1_config_t, width_chars, UI_LOWER_WIDTH_CHAR);
    xram0_struct_set(XRAM_UI_LOWER_CONFIG, vga_mode1_config_t, height_chars, UI_LOWER_HEIGHT_CHAR);
    xram0_struct_set(XRAM_UI_LOWER_CONFIG, vga_mode1_config_t, xram_data_ptr, XRAM_UI_LOWER);
    xram0_struct_set(XRAM_UI_LOWER_CONFIG, vga_mode1_config_t, xram_palette_ptr, XRAM_DEFAULT_PALETTE);
    xram0_struct_set(XRAM_UI_LOWER_CONFIG, vga_mode1_config_t, xram_font_ptr, XRAM_DEFAULT_FONT);

    RIA.addr0 = XRAM_UI_UPPER;
    RIA.step0 = 1;
    for (i = 0; i < UI_UPPER_SIZE; i++)
    {
        RIA.rw0 = 0; // data
        RIA.rw0 = 0; // fgbg color index
    }
    message = "Upper UI elements       they can go here";
    RIA.addr0 = XRAM_UI_UPPER;
    RIA.step0 = 1;
    for (i = 0; i < UI_UPPER_SIZE; i++)
    {
        c = message[i];
        RIA.rw0 = c;
        RIA.rw0 = MODE1_BG_FG(ANSI_TRANSPARENT, ANSI_BRIGHT_CYAN);
    }

    RIA.addr0 = XRAM_UI_PAUSE;
    RIA.step0 = 1;
    for (i = 0; i < UI_PAUSE_SIZE; i++)
    {
        RIA.rw0 = 0; // data
        RIA.rw0 = 0; // fgbg color index
    }
    RIA.addr0 = XRAM_UI_MESSAGE;
    RIA.step0 = 1;
    for (i = 0; i < UI_MESSAGE_SIZE; i++)
    {
        RIA.rw0 = 0; // data
        RIA.rw0 = 0; // fgbg color index
    }
    RIA.addr0 = XRAM_UI_LOWER;
    RIA.step0 = 1;
    for (i = 0; i < UI_LOWER_SIZE; i++)
    {
        RIA.rw0 = 0; // data
        RIA.rw0 = 0; // fgbg color index
    }

    message = "Lower UI elements       they can go here";
    RIA.addr0 = XRAM_UI_LOWER;
    RIA.step0 = 1;
    for (i = 0; i < UI_LOWER_SIZE; i++)
    {
        c = message[i];
        RIA.rw0 = c;
        RIA.rw0 = MODE1_BG_FG(ANSI_BRIGHT_BLACK, ANSI_WHITE);
    }

    xreg_vga_mode1(MODE1_4BPP, XRAM_UI_UPPER_CONFIG, VGA_PLANE_UI, UI_UPPER_SCANLINE_START, UI_UPPER_SCANLINE_END);
    xreg_vga_mode1(MODE1_4BPP, XRAM_UI_PAUSE_CONFIG, VGA_PLANE_UI, UI_PAUSE_SCANLINE_START, UI_PAUSE_SCANLINE_END);
    xreg_vga_mode1(MODE1_4BPP, XRAM_UI_MESSAGE_CONFIG, VGA_PLANE_UI, UI_MESSAGE_SCANLINE_START, UI_MESSAGE_SCANLINE_END);
    xreg_vga_mode1(MODE1_4BPP, XRAM_UI_LOWER_CONFIG, VGA_PLANE_UI, UI_LOWER_SCANLINE_START, UI_LOWER_SCANLINE_END);
}

void ui_show_message(char *message)
{
    uint8_t i;
    char c;

    RIA.addr0 = XRAM_UI_MESSAGE + (12 * 2);
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

    RIA.addr0 = XRAM_UI_PAUSE;
    RIA.step0 = 1;
    for (i = 0; i < UI_PAUSE_SIZE; i++)
    {
        c = message[i];
        RIA.rw0 = c;
        RIA.rw0 = MODE1_BG_FG(ANSI_TRANSPARENT, ANSI_BRIGHT_WHITE);
    }
}

void ui_clear_pause(void)
{
    uint8_t i;

    RIA.addr0 = XRAM_UI_PAUSE;
    RIA.step0 = 1;
    for (i = 0; i < UI_PAUSE_SIZE; i++)
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
