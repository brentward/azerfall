#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>
#include <stdbool.h>

#include "../entity/player.h"

void event_check(player_t *player);
void event_init(void);

#endif // EVENT_H
