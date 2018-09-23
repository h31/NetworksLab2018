cmake_minimum_required(VERSION 2.8)
project(client_windows)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -lws2_32")

set(SOURCE_FILES main.c)
add_executable(client_windows ${SOURCE_FILES})