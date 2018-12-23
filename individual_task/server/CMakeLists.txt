cmake_minimum_required(VERSION 2.8)
project(BugTracker C)

find_package(Threads)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS_pthread)

add_executable(BugTracker src/main.c src/file_manager/fileManager.c src/file_manager/fileManager.h src/client_actions/testerActions.c src/client_actions/testerActions.h src/structures.h src/socket_functions/socketFunctions.c src/socket_functions/socketFunctions.h src/menu_listener/optionsMenuListener.c src/menu_listener/optionsMenuListener.h src/includes.h src/client_actions/developerActions.h src/client_actions/developerActions.c src/client_actions/commonActions.h src/client_actions/commonActions.c src/server_actions/serverThreads.c src/server_actions/serverThreads.h src/server_actions/serverActions.c src/server_actions/serverActions.h)

target_link_libraries(BugTracker ${CMAKE_THREAD_LIBS_INIT})