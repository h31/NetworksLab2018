cmake_minimum_required(VERSION 2.8) 
project(server_windows) 

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") 

file(GLOB SOURCE_FILES 
"main.cpp"
) 
add_executable(client_linux ${SOURCE_FILES})