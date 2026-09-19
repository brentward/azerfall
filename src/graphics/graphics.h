#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

typedef struct {
    int16_t screen_x;
    int16_t screen_y;
} ScreenPosition;

typedef struct {
    int x_pos_px;
    int y_pos_px;
    unsigned xram_sprite_ptr;
    unsigned palette_ptr;
} SpriteConfig;

typedef struct {
    int x_pos_px;
    int y_pos_px;
} SpriteConfigPosition;

typedef struct {
    int x_pos_px;
    int y_pos_px;
    unsigned xram_sprite_ptr;
} SpriteConfigAnimation;



#endif // GRAPHICS_H