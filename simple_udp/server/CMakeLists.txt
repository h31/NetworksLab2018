cmake_minimum_required(VERSION 3.8)
project(server_windows)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c)
add_executable(server_windows ${SOURCE_FILES})
target_link_libraries(server_windows wsock32)