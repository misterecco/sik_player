cmake_minimum_required(VERSION 3.5)
project(sik_player)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -std=gnu99 -pthread -fsanitize=address -fsanitize=undefined")

set(SOURCE_FILES_PLAYER player.h player.c player_network.c player_initialize.c player_local.c player_parse.c)

set(SOURCE_FILES_MASTER master.h master.c master_telnet_list.c master_telnet.c master_initialize.c master_ssh.c master_time.c common.c common.h master_parse.c master_player_list.c master_commands.c)

set(SOURCE_FILES_COMMON err.h err.c common.h common.c)

add_executable(player_files ${SOURCE_FILES_PLAYER} ${SOURCE_FILES_COMMON})
add_executable(master_files ${SOURCE_FILES_MASTER} ${SOURCE_FILES_COMMON})