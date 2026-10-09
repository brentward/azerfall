cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)

project(MY-RP6502-PROJECT C CXX ASM)

add_executable(azerfall)
rp6502_map(azerfall src/xram.h "XRAM_.*")
rp6502_asset(azerfall help src/help.txt)
rp6502_asset(azerfall Z3LIGHTW.BIN assets/music/Z3LIGHTW.BIN)
rp6502_executable(azerfall DATA default RESET default)
rp6502_web(azerfall)
target_sources(azerfall PRIVATE
    src/main.c
    generated/world_tiles.c
    generated/world_tiles_worldmap_map.c
    src/game/game.c
    generated/player_sprites.c
    src/entity/player.c
    src/input/input.c
    src/world/collision.c
    src/world/map.c
    src/object/object.c
    generated/object_sprites.c
    src/audio/music.c
    src/audio/sound.c
    src/game/ui.c
    src/entity/npc.c
    src/entity/entity.c
    src/graphics/graphics.c
    generated/npc_sprites.c
    src/game/event.c
    src/entity/monster.c
    generated/monster_sprites.c
)
