#include "weapon.h"

#include <stdint.h>
#include <string.h>

#include "../xram.h"
#include "../entity/player.h"

void weapon_sprite_init(void)
{
    int i;
    const uint8_t *pixels = weapon_sprites;
    const uint16_t *palette = weapon_sprites_palette;


    RIA.addr0 = XRAM_WEAPON_IMAGES;
    RIA.step0 = 1;
    for (i = 0; i < WEAPON_SPRITES_TOTAL_BYTES; i++, pixels++)
    {
        RIA.rw0 = *pixels;
    }

    RIA.addr0 = XRAM_WEAPON_PALETTE;
    for (i = 0; i < WEAPON_SPRITES_PALETTE_COUNT; i++, palette++)
    {
        RIA.rw0 = (uint8_t)*palette;
        RIA.rw0 = (uint8_t)(*palette >> 8);
    }

}

void weapon_normal_sword_init(player_t *player)
{
    weapon_t *sword;

    sword = &player->current_weapon;
    sword->attack_value = 1;
    sword->weapon_type = SWORD_NORMAL;


    

    xram0_struct_set(XRAM_SPRITE_CONFIG(WEAPON_SPRITE_CONFIG_SLOT), vga_mode5_sprite_t, x_pos_px, SCREEN_WIDTH / 2);
    xram0_struct_set(XRAM_SPRITE_CONFIG(WEAPON_SPRITE_CONFIG_SLOT), vga_mode5_sprite_t, y_pos_px, 5 );
    xram0_struct_set(XRAM_SPRITE_CONFIG(WEAPON_SPRITE_CONFIG_SLOT), vga_mode5_sprite_t, xram_sprite_ptr, XRAM_WEAPON_IMAGES + WEAPON_IMAGE0 * WEAPON_SPRITES_BYTES_PER_SPRITE);
    xram0_struct_set(XRAM_SPRITE_CONFIG(WEAPON_SPRITE_CONFIG_SLOT), vga_mode5_sprite_t, palette_ptr, XRAM_WEAPON_PALETTE);
}

void weapon_wood_shield_init(player_t *player)
{
    shield_t *shield;
    
    shield = &player->current_shield;
    shield->defense_value = 1;
    shield->shield_type = SHIELD_WOOD;

    xram0_struct_set(XRAM_SPRITE_CONFIG(SHIELD_SPRITE_CONFIG_SLOT), vga_mode5_sprite_t, x_pos_px, SCREEN_WIDTH - 5 - TILE_SIZE);
    xram0_struct_set(XRAM_SPRITE_CONFIG(SHIELD_SPRITE_CONFIG_SLOT), vga_mode5_sprite_t, y_pos_px, 5 );
    xram0_struct_set(XRAM_SPRITE_CONFIG(SHIELD_SPRITE_CONFIG_SLOT), vga_mode5_sprite_t, xram_sprite_ptr, XRAM_WEAPON_IMAGES + WEAPON_IMAGE1 * WEAPON_SPRITES_BYTES_PER_SPRITE);
    xram0_struct_set(XRAM_SPRITE_CONFIG(SHIELD_SPRITE_CONFIG_SLOT), vga_mode5_sprite_t, palette_ptr, XRAM_WEAPON_PALETTE);
}