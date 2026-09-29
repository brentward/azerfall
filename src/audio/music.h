#ifndef MUSIC_H
#define MUSIC_H

#include <stdbool.h>

#include "../game/game.h"

/* Call sound_init once before loading music; it owns the shared OPL device. */
void music_init(game_t *game, const char *path, bool loop);
/* Configure before playback. Offset is a four-byte BIN packet boundary. */
bool music_set_loop_offset(game_t *game, uint16_t offset);
/* Audition helper: fast-forward from the beginning, preserving register state. */
void music_skip_to_frame(game_t *game, uint16_t frame);
void music_update(game_t *game);
void music_pause(void);
void music_resume(void);

#endif // MUSIC_H
