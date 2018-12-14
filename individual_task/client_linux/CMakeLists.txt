cmake_minimum_required(VERSION 3.12)
project(client_linux C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(SOURCE_FILES main.c sockets/socket.h sockets/connect_socket.c sockets/close_socket.c sockets/read_socket.c sockets/send_request.c sockets/parse_request.c sockets/response.c data/data.h data/pack_data.c data/calculate_data.c)
add_executable(client_linux ${SOURCE_FILES})