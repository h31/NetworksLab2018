cmake_minimum_required(VERSION 3.12)
project(server)

find_package(Threads)
find_package(PostgreSQL REQUIRED)
include_directories(/usr/include/postgresql)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.65.0 REQUIRED COMPONENTS log)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

include_directories(./)
include_directories(../utils)
file(GLOB SOURCE_FILES "*.cpp" ../utils/*.cpp ../utils/*.h)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} Threads::Threads)
target_link_libraries(${PROJECT_NAME} pq)