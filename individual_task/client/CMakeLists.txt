cmake_minimum_required(VERSION 2.8)
project(client_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

file(GLOB SOURCE
	"*.c" 
	"sockets/*.c"
)

add_executable(client_linux ${SOURCE})
