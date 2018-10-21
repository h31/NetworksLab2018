cmake_minimum_required(VERSION 2.8)
project(server_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(SOURCE_FILES ../util_linux/util_linux.h ../util_linux/util_linux.c main.c )
add_executable(server_linux ${SOURCE_FILES})
