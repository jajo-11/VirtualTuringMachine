cmake_minimum_required(VERSION 3.0)
project(VirtualTuringMachine C)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory")

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

add_subdirectory(src/utils)
add_subdirectory(src)