#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>

#include "../game/entity.h"
#include "../object/object.h"


void collision_check_tiles(entity_t *entity, int16_t dx, int16_t dy);
uint8_t collision_check_object(entity_t *entity, object_t *objects, int16_t dx, int16_t dy);

#endif // COLLISION_H