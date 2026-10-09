#ifndef RP6502_COMPAT_H
#define RP6502_COMPAT_H

#include <rp6502.h>
#include <stddef.h>

#define RIA_READY_TX_BIT 0x80
#define RIA_READY_RX_BIT 0x40
#define RIA_BUSY_BIT 0x80
#define RIA_OP_ZXSTACK RIA_OP_DROP_XSTACK

#define phi2() ((int)ria_attr_get(RIA_ATTR_PHI2_KHZ))
#define code_page(cp)                                                    \
    ((cp) ? (void)ria_attr_set((cp), RIA_ATTR_CODE_PAGE) : (void)0,      \
     (int)ria_attr_get(RIA_ATTR_CODE_PAGE))
#define lrand() ria_attr_get(RIA_ATTR_LRAND)

#define xreg_ria_keyboard(...) xreg(0, 0, 0, __VA_ARGS__)
#define xreg_ria_mouse(...) xreg(0, 0, 1, __VA_ARGS__)
#define xreg_ria_gamepad(...) xreg(0, 0, 2, __VA_ARGS__)
#define xreg_ria_tablet(...) xreg(0, 0, 3, __VA_ARGS__)
#define xreg_vga_canvas(...) xreg(1, 0, 0, __VA_ARGS__)
#define xreg_vga_mode(...) xreg(1, 0, 1, __VA_ARGS__)

/* Bit tests, not ==, avoid a cc65 warning about constant comparisons.
   The casts avoid conversion warnings from the unused branches. */
#define xram_struct_set_(n, a, size, v)                                  \
    ((size) & 1 ? xram##n##_poke8(a, (unsigned char)(v))                 \
     : (size) & 2 ? xram##n##_poke16(a, (unsigned)(v))                  \
     : (xram##n##_poke16(a, (unsigned)(v)),                              \
        xram##n##_poke16((a) + 2, (unsigned)((unsigned long)(v) >> 16))))
#define xram0_struct_set(addr, type, member, val)                        \
    xram_struct_set_(0, (unsigned)(addr) + offsetof(type, member),       \
                     sizeof(((type *)0)->member), val)
#define xram1_struct_set(addr, type, member, val)                        \
    xram_struct_set_(1, (unsigned)(addr) + offsetof(type, member),       \
                     sizeof(((type *)0)->member), val)

typedef struct {
    unsigned char x_wrap, y_wrap;
    int x_pos_px, y_pos_px, width_chars, height_chars;
    unsigned xram_data_ptr, xram_palette_ptr, xram_font_ptr;
} vga_mode1_config_t;

typedef struct {
    unsigned char x_wrap, y_wrap;
    int x_pos_px, y_pos_px, width_tiles, height_tiles;
    unsigned xram_data_ptr, xram_palette_ptr, xram_tile_ptr;
} vga_mode2_config_t;

typedef struct {
    unsigned char x_wrap, y_wrap;
    int x_pos_px, y_pos_px, width_px, height_px;
    unsigned xram_data_ptr, xram_palette_ptr;
} vga_mode3_config_t;

typedef struct {
    int x_pos_px, y_pos_px;
    unsigned xram_sprite_ptr;
    unsigned char log_size, has_opacity_metadata;
} vga_mode4_sprite_t;

typedef struct {
    int transform[6];
    int x_pos_px, y_pos_px;
    unsigned xram_sprite_ptr;
    unsigned char log_size, has_opacity_metadata;
} vga_mode4_asprite_t;

typedef struct {
    int x_pos_px, y_pos_px;
    unsigned xram_sprite_ptr, palette_ptr;
} vga_mode5_sprite_t;

#endif /* RP6502_COMPAT_H */