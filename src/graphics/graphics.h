#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#include "../object/object.h"
#include "../entity/entity.h"
#include "../entity/player.h"

typedef struct {
    int16_t screen_x;
    int16_t screen_y;
} screen_position_t;

screen_position_t get_screen_position_object(object_t *obj, player_t *player);
screen_position_t get_screen_position_entity(entity_t *entity, player_t *player);

#endif // GRAPHICS_H