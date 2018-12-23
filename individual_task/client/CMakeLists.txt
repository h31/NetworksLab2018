cmake_minimum_required(VERSION 2.8)
project(BugTrackerClient C)

set(CMAKE_C_STANDARD 99)

add_executable(BugTrackerClient main.c includes.h socketFunctions.h socketFunctions.c)