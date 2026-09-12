#ifndef COLLISION_H
#define COLLISION_H

#include <stdint.h>
#include <stdbool.h>
#include "../game/entity.h"
#include "../game/game.h"
#include "map.h"

bool collision_check_tiles(const Entity *entity);

#endif // COLLISION_H