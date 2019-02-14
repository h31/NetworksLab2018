cmake_minimum_required(VERSION 3.8)
project(TCPClient)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread")

set(SOURCE_FILES main.c)
add_executable(TCPClient ${SOURCE_FILES})
target_link_libraries(TCPClient Ws2_32.lib Mswsock.lib AdvApi32.lib)