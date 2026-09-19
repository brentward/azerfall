#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>

#include "../game/entity.h"
#include "../object/object.h"


uint8_t collision_check_tiles(const Entity *entity);
uint8_t collision_check_object(const Entity *entity, GameObject *objects);

#endif // COLLISION_H