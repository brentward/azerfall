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
    // // Move X first, then test Y from the resulting position.
    // // Keep facing the input direction while sliding along an obstacle.
    // entity->collision_on = false;
    // collision_check_tiles(entity, dx, 0);
    // index_x = collision_check_object(entity, objects, dx, 0);
    // blocked_x = entity->collision_on;
    // if (!blocked_x)
    //     entity->world_x += dx;

    // entity->collision_on = false;
    // collision_check_tiles(entity, 0, dy);
    // index_y = collision_check_object(entity, objects, 0, dy);
    // if (!entity->collision_on)
    //     entity->world_y += dy;
    // entity->collision_on = entity->collision_on || blocked_x;

    // entity_animation_update(entity);
    // Move X first, then test Y from the resulting position.
    // Keep facing the input direction while sliding along an obstacle.
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
    
    entity_animation_update(entity);

}

// Do not call with the entity_t in player_t types
static void entity_animation_update(entity_t *entity)
{
    int sprite_index;

    entity->animation_timer++;
    if (entity->animation_timer >= 12) { // Example timer threshold
        entity->animation_timer = 0;
        entity->animation_frame = (entity->animation_frame + 1) % 2; // Example animation frame update
    }
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
            entity->xram_sprite_ptr =  XRAM_MONSTER_IMAGES + sprite_index * BYTES_PER_SPRITE;
            break;
        default:
            sprite_index = OLDMAN_DIR1_FRAME0 + entity->animation_frame;
            entity->xram_sprite_ptr =  XRAM_NPC_IMAGES + sprite_index * BYTES_PER_SPRITE;
            break;
    }

    // switch (entity->state) {
    //     case ENTITY_IDLE:
    //         sprite_index = entity->direction * 2 + entity->type;
    //         break;
    //     case ENTITY_WALKING:
    //         sprite_index = entity->direction * 2 + entity->animation_frame + entity->type;
    //         // Animation frame is updated in player_update based on timer
    //         break;
    //     case ENTITY_ATTACKING:
    //         // Handle attacking animation
    //         break;
    //     case ENTITY_DYING:
    //         // Handle dying animation
    //         break;
    // }
    
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
}
