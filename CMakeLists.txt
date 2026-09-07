cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)

project(MY-RP6502-PROJECT C CXX ASM)

add_executable(azerfall)
rp6502_asset(azerfall help src/help.txt)
rp6502_executable(azerfall DATA default RESET default)
target_sources(azerfall PRIVATE
    src/main.c
    src/grass_path.c
    src/game.c
    src/player_sprites.c
)
