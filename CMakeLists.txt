cmake_minimum_required(VERSION 3.5)
project(sik_player)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -std=gnu99 -pthread")

set(SOURCE_FILES_PLAYER player/player.h player/player.c player/player_network.c player/player_initizalize.c player/player_local.c player/player_parse.c)

set(SOURCE_FILES_MASTER master/master.h master/master.c)

set(SOURCE_FILES_COMMON libs/err.h libs/err.c)

add_executable(player ${SOURCE_FILES_PLAYER} ${SOURCE_FILES_COMMON})
add_executable(master ${SOURCE_FILES_MASTER} ${SOURCE_FILES_COMMON})