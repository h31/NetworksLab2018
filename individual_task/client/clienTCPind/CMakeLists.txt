cmake_minimum_required(VERSION 3.13)
project(clienTCPind C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread")

add_executable(clienTCPind main.c)