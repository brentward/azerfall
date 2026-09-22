#ifndef MUSIC_H
#define MUSIC_H

#include <stdbool.h>

#include "../game/game.h"

void music_init(Game *game, const char *path, bool loop);
void music_update(Game *game);

#endif // MUSIC_H