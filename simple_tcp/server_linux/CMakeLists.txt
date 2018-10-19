cmake_minimum_required(VERSION 2.8)
project(server_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -lpthread")

set(SOURCE_FILES main.c)
add_executable(server_linux ${SOURCE_FILES})