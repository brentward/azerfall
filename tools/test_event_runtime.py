"""Exercise map event activation using the actual C event implementation."""
import shutil
import unittest

from tools.test_music_runtime import ROOT, run_c


@unittest.skipUnless(shutil.which("cl65") and shutil.which("sim65"),
                     "requires cc65/sim65")
class EventRuntimeTests(unittest.TestCase):
    def test_tile_overlap_direction_reentry_and_healing(self):
        source = (ROOT / "src/game/event.c").read_text()
        source = "\n".join(line for line in source.splitlines()
                           if not line.startswith("#include"))
        prelude = r'''
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#define TILE_SIZE 16
typedef enum { DIR_UP, DIR_RIGHT, DIR_LEFT, DIR_ANY } direction_t;
typedef enum { GAME_STATE_PLAY, GAME_STATE_DIALOGUE } game_state_t;
typedef struct { int16_t x, y; uint8_t width, height; } hitbox_t;
typedef struct {
    int16_t world_x, world_y;
    hitbox_t hitbox;
    direction_t direction;
    uint8_t life, max_life;
} entity_t;
typedef struct { entity_t entity; } player_t;
static struct { bool interact_pressed; } input_state;
static unsigned dialogues;
static game_state_t state;
static void game_state_set(game_state_t next) { state = next; }
static void ui_show_dialogue(const char *message) { ++dialogues; }
'''
        harness = r'''
int main(void)
{
    player_t player;
    player.entity.hitbox.x = 2;
    player.entity.hitbox.y = 4;
    player.entity.hitbox.width = 8;
    player.entity.hitbox.height = 8;
    player.entity.max_life = 12;
    player.entity.life = 12;
    player.entity.world_x = 432;
    player.entity.world_y = 256;
    player.entity.direction = DIR_LEFT;
    event_init();
    event_check(&player);
    assert(dialogues == 0);
    player.entity.direction = DIR_RIGHT;
    event_check(&player);
    assert(dialogues == 1 && player.entity.life == 11);
    assert(state == GAME_STATE_DIALOGUE);
    event_check(&player);
    assert(dialogues == 1 && player.entity.life == 11);
    player.entity.world_x = 400;
    event_check(&player);
    player.entity.world_x = 432;
    player.entity.life = 0;
    event_check(&player);
    assert(dialogues == 2 && player.entity.life == 0);
    player.entity.world_x = 368;
    player.entity.world_y = 192;
    player.entity.direction = DIR_UP;
    event_check(&player);
    assert(dialogues == 2);
    input_state.interact_pressed = true;
    event_check(&player);
    assert(dialogues == 3 && player.entity.life == 12);
    assert(state == GAME_STATE_DIALOGUE && !input_state.interact_pressed);
    event_check(&player);
    assert(dialogues == 3);
    return 0;
}
'''
        run_c(prelude + source + harness, {})
