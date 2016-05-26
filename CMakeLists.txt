cmake_minimum_required(VERSION 3.5)
project(sik_player)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -std=gnu99 -DDEBUG -pthread")

set(SOURCE_FILES_PLAYER player.h  player.c player_network.c player_initizalize.c player_local.c player_parse.c)

set(SOURCE_FILES_COMMON common.h common.c err.h err.c)

add_executable(player ${SOURCE_FILES_PLAYER} ${SOURCE_FILES_COMMON})