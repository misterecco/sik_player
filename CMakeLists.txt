cmake_minimum_required(VERSION 3.5)
project(sik_player)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -DDEBUG")

set(SOURCE_FILES_PLAYER player.cpp)

add_executable(player ${SOURCE_FILES_PLAYER} )