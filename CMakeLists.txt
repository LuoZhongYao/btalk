project(btalk)
cmake_minimum_required(VERSION 3.8)

include(config.cmake)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

add_subdirectory(src)
add_subdirectory(soc)
add_subdirectory(external)
add_subdirectory(bluetooth)
