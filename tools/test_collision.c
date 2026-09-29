#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/world/collision.h"
#include "../src/game/game.h"

static uint8_t wall_mode;
static GameObject objects[OBJECT_COUNT];

uint8_t map_tile_is_solid(uint16_t col, uint16_t row)
{
    if (wall_mode == 1) return col == 1;
    if (wall_mode == 2) return row == 1;
    if (wall_mode == 3) return col == 1 && row == 1;
    return 0;
}

static void step(Entity *e, int16_t dx, int16_t dy)
{
    e->collision_on = false;
    collision_check_tiles(e, dx, 0);
    collision_check_object(e, objects, dx, 0);
    if (!e->collision_on) e->world_x += dx;
    e->collision_on = false;
    collision_check_tiles(e, 0, dy);
    collision_check_object(e, objects, 0, dy);
    if (!e->collision_on) e->world_y += dy;
}

int main(void)
{
    Entity e;
    uint8_t i;
    memset(&e, 0, sizeof e);
    memset(objects, 0, sizeof objects);
    e.hitbox.width = e.hitbox.height = 1;
    for (i = 0; i < OBJECT_COUNT; ++i) {
        objects[i].world_x = objects[i].world_y = -500;
        objects[i].hitbox.width = objects[i].hitbox.height = 1;
    }
    wall_mode = 1;
    e.world_x = e.world_y = 15;
    step(&e, 1, 1);
    assert(e.world_x == 15 && e.world_y == 16);
    wall_mode = 2;
    e.world_x = e.world_y = 15;
    step(&e, 1, 1);
    assert(e.world_x == 16 && e.world_y == 15);
    wall_mode = 3;
    e.world_x = e.world_y = 15;
    step(&e, 1, 1);
    assert(e.world_x == 16 && e.world_y == 15);
    wall_mode = 0;
    e.world_x = e.world_y = 15;
    step(&e, 1, 1);
    assert(e.world_x == 16 && e.world_y == 16);
    e.world_x = e.world_y = 0;
    step(&e, -1, -1);
    assert(e.world_x == 0 && e.world_y == 0);
    /* A nonblocking pickup must not mask a solid object behind it. */
    objects[0].world_x = objects[1].world_x = 16;
    objects[0].world_y = objects[1].world_y = 15;
    objects[1].collision = true;
    e.world_x = e.world_y = 15;
    e.collision_on = false;
    assert(collision_check_object(&e, objects, 1, 0) == 0);
    assert(e.collision_on);
    step(&e, 1, 1);
    assert(e.world_x == 15 && e.world_y == 16);
    puts("Collision checks passed");
    return 0;
}