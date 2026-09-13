#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>

#include "../game/entity.h"

bool collision_check_tiles(const Entity *entity);

#endif // COLLISION_H