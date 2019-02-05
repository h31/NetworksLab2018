cmake_minimum_required(VERSION 3.8)
project(tcp_windows_client)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c)
add_executable(tcp_windows_client ${SOURCE_FILES})
target_link_libraries(tcp_windows_client Ws2_32.lib Mswsock.lib AdvApi32.lib)