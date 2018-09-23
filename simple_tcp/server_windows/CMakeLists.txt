cmake_minimum_required(VERSION 2.8)
project(server_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -lws2_32")

set(SOURCE_FILES main.c)
add_executable(server_linux ${SOURCE_FILES})