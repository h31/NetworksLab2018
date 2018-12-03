cmake_minimum_required(VERSION 2.8)
project(server_windows)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
link_libraries(ws2_32 wsock32)

set(SOURCE_FILES main.c)
add_executable(server_windows.result ${SOURCE_FILES})
