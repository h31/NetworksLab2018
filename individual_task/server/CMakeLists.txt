cmake_minimum_required(VERSION 3.8)
project(TCPServer)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread")

set(SOURCE_FILES main.c)
add_executable(TCPServer ${SOURCE_FILES})