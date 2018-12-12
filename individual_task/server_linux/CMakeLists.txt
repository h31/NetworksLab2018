cmake_minimum_required(VERSION 3.12)
project(server_linux C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

find_package (Threads)
set(SOURCE_FILES main.c sockets/socket.h sockets/listen_socket.c sockets/close_socket.c sockets/get_request.c sockets/read_socket.c sockets/response_request.c users_and_data/work.h users_and_data/create_data_storage.c users_and_data/read_file.c users_and_data/write_file.c users_and_data/create_user.c users_and_data/check_user.c users_and_data/delete_user.c users_and_data/create_session.c users_and_data/delete_session.c users_and_data/generate_token.c users_and_data/get_session_client.c users_and_data/clear_data.c users_and_data/calculate_data.c users_and_data/find_maxprime.c users_and_data/get_list_of_primes.c users_and_data/retrieve_data.c users_and_data/pack_data.c)
add_executable(server_linux ${SOURCE_FILES})

target_link_libraries (server_linux ${CMAKE_THREAD_LIBS_INIT})