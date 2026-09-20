#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>

#include "../game/entity.h"
#include "../object/object.h"


void collision_check_tiles(Entity *entity);
uint8_t collision_check_object(Entity *entity, GameObject *objects);

#endif // COLLISION_H