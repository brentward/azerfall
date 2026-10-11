#include "entity.h"

#include "../xram.h"
#include "../input/input.h"
#include "npc.h"
#include "monster.h"
#include "../graphics/graphics.h"
#include "../world/collision.h"

static void entity_animation_update(entity_t *entity);

// Do not call with the entity_t in player_t types
void entity_update(entity_t *entity, struct player_t *player, struct object_t *objects, npc_t *npcs, entity_t *monsters)
{
    int16_t dx;
    int16_t dy;

    int move_x;
    int move_y;

    if (entity->state == ENTITY_DEAD)
        return;
    if (entity->state == ENTITY_DYING)
    {
        entity_animation_update(entity);
        return;
    }

    switch (entity->type)
    {
        case ENTITY_NPC_OLDMAN:
            npc_oldman_update(entity);
            break;

        case ENTITY_NPC_MERCHANT:
            npc_merchant_update(entity);
            break;

        case ENTITY_MONSTER_GREENSLIME:
            monster_greenslime_update(entity);
        
        default:
            break;

    }

    switch (entity->direction)
    {
        case DIR_UP:
            move_x = 0;
            move_y = -1;
            break;
        
        case DIR_DOWN:
            move_x = 0;
            move_y = 1;
            break;
        
        case DIR_LEFT:
            move_x = -1;
            move_y = 0;
            break;

        case DIR_RIGHT:
            move_x = 1;
            move_y = 0;
            break;
        
        default:
            move_x = 0;
            move_y = 0;
            break;
    }
    dx = move_x * entity->speed;
    dy = move_y * entity->speed;
    entity->state = (dx != 0 || dy != 0) ? ENTITY_WALKING : ENTITY_IDLE;
    entity->collision_on = false;
    if (entity->state == ENTITY_IDLE)
    {
        entity_animation_update(entity);
        return;
    }
    entity->collision_on = false;
    collision_check_tiles(entity, dx, dy);
    if (!entity->collision_on)
        collision_check_object(entity, objects, dx, dy);
    if (!entity->collision_on)
        collision_check_npcs(entity, npcs, dx, dy);
    if (!entity->collision_on)
        collision_check_monsters(entity, monsters, dx, dy);
    if (!entity->collision_on)
    {
        collision_check_player(entity, player, dx, dy);
        if (entity->collision_on && entity->type == ENTITY_MONSTER_GREENSLIME)
            player_damage(player, entity->attack);
    }
        

    if (!entity->collision_on)
    {
        entity->world_x += dx;
        entity->world_y += dy;
    }
    if (entity->invincible)
    {
        if (++entity->invincible_counter >= 40)
        {
            entity->invincible = false;
            entity->invincible_counter = 0;
        }
    }

    
    entity_animation_update(entity);

}

// Do not call with the entity_t in player_t types
static void entity_animation_update(entity_t *entity)
{
    int sprite_index;

    if (entity->state == ENTITY_DYING)
    {
        /* Display each death frame for twelve updates, then stop. */
        if (entity->animation_timer >= 12)
        {
            entity->animation_timer = 0;
            if (++entity->animation_frame >= MONSTER_DYING_ANIMATION_FRAMES)
            {
                entity->state = ENTITY_DEAD;
                entity->animation_frame = 0;
                entity->action_lock_counter = 0;
                xram0_struct_set(XRAM_SPRITE_CONFIG(entity->config_slot), vga_mode5_sprite_t, x_pos_px, -16);
                return;
            }
        }
        sprite_index = MONSTER_DYING_ANIMATION_LOOKUP[entity->animation_frame];
        entity->animation_timer++;
        entity->xram_sprite_ptr = XRAM_MONSTER_IMAGES + sprite_index * BYTES_PER_SPRITE;
                 
    } else 
    {

        switch (entity->type)
        {
            case ENTITY_NPC_OLDMAN:
                entity->animation_timer++;
                if (entity->animation_timer >= 12) { // Example timer threshold
                    entity->animation_timer = 0;
                    entity->animation_frame = (entity->animation_frame + 1) % 2; // Example animation frame update
                }
                switch (entity->direction)
                {
                    case DIR_UP:
                        sprite_index = OLDMAN_DIR1_FRAME0 + entity->animation_frame;
                        break;
                    case DIR_DOWN:
                        sprite_index = OLDMAN_DIR0_FRAME0 + entity->animation_frame;
                        break;
                    case DIR_LEFT:
                        sprite_index = OLDMAN_DIR2_FRAME0 + entity->animation_frame;
                        break;
                    case DIR_RIGHT:
                        sprite_index = OLDMAN_DIR3_FRAME0 + entity->animation_frame;
                        break;
                    default:
                        sprite_index = OLDMAN_DIR0_FRAME0 + entity->animation_frame;
                        break;
                }
                entity->xram_sprite_ptr =  XRAM_NPC_IMAGES + sprite_index * BYTES_PER_SPRITE;
                break;
            case ENTITY_NPC_MERCHANT:
                entity->animation_timer++;
                if (entity->animation_timer >= 12) { // Example timer threshold
                    entity->animation_timer = 0;
                    entity->animation_frame = (entity->animation_frame + 1) % 2; // Example animation frame update
                }

                sprite_index = MERCHANT_DIR0_FRAME0 + entity->animation_frame;
                entity->xram_sprite_ptr =  XRAM_NPC_IMAGES + sprite_index * BYTES_PER_SPRITE;
                break;
            case ENTITY_MONSTER_GREENSLIME:
                entity->animation_timer++;
                if (entity->animation_timer >= 12) { // Example timer threshold
                    entity->animation_timer = 0;
                    entity->animation_frame = (entity->animation_frame + 1) % 4; // Example animation frame update
                }

                switch (entity->animation_frame)
                {
                    case 0:
                        sprite_index = GREENSLIME_FRAME0;
                        break;
                    case 1:
                        sprite_index = GREENSLIME_FRAME1;
                        break;
                    case 2:
                        sprite_index = GREENSLIME_FRAME2;
                        break;
                    case 3:
                        sprite_index = GREENSLIME_FRAME3;
                        break;
                    default:
                        sprite_index = GREENSLIME_FRAME0;
                        break;

                }
                if (entity->invincible && entity->animation_frame %2 == 0)
                    entity->xram_palette_ptr = XRAM_WORLD_PALETTE;
                else
                    entity->xram_palette_ptr = XRAM_MONSTER_PALETTE;



                entity->xram_sprite_ptr =  XRAM_MONSTER_IMAGES + sprite_index * BYTES_PER_SPRITE;
                break;
            default:
                sprite_index = OLDMAN_DIR1_FRAME0 + entity->animation_frame;
                entity->xram_sprite_ptr =  XRAM_NPC_IMAGES + sprite_index * BYTES_PER_SPRITE;
                break;
        }
    }
    // if (entity->invincible && entity->animation_frame %2 == 0)
    // {
    //     entity->xram_sprite_ptr = XRAM_MONSTER_IMAGES + MONSTER_EMPTY_FRAME * BYTES_PER_SPRITE;
    // }

}

void entity_damage(entity_t *entity, uint8_t attack)
{
    if (!entity->invincible && attack > 0 && entity->life > 0)
    {
        entity->life = attack >= entity->life ? 0 : entity->life - attack;
        entity->invincible = true;
        entity->invincible_counter = 0;
    }
}


void entity_prepare_draw(entity_t *entity, player_t *player)
{
    screen_position_t screen_position;


    screen_position = get_screen_position_entity(entity, player);

    entity->screen_x = screen_position.screen_x;
    entity->screen_y = screen_position.screen_y;
    /* entity_animation_update already selected the NPC's direction and frame. */
}


void entity_draw(entity_t *entity, uint8_t config_slot)
{
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, x_pos_px, entity->screen_x);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, y_pos_px, entity->screen_y);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, xram_sprite_ptr, entity->xram_sprite_ptr);
    xram0_struct_set(XRAM_SPRITE_CONFIG(config_slot), vga_mode5_sprite_t, palette_ptr, entity->xram_palette_ptr);
}
