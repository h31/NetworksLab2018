cmake_minimum_required(VERSION 2.8)
project(client_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")

set(SOURCE_FILES main.c)
add_executable(client_linux ${SOURCE_FILES})
