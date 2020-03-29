project(btalk)
cmake_minimum_required(VERSION 3.8)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(/usr/arm-none-eabi/include/)

include(config.cmake)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,-T${CMAKE_SOURCE_DIR}/soc/gcc_arm.ld")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostartfiles")
configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

add_subdirectory(src)
add_subdirectory(soc)
#add_subdirectory(sys)
add_subdirectory(external)
add_subdirectory(bluetooth)
