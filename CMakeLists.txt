cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)

project(MY-RP6502-PROJECT C CXX ASM)

add_executable(azerfall)
option(MUSIC_AUDITION "Start music five seconds before the loop boundary" OFF)
if(MUSIC_AUDITION)
    target_compile_definitions(azerfall PRIVATE MUSIC_AUDITION)
endif()
rp6502_asset(azerfall help src/help.txt)
rp6502_asset(azerfall Z3LIGHTW.BIN assets/music/Z3LIGHTW.BIN)
rp6502_executable(azerfall DATA default RESET default)
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
)
