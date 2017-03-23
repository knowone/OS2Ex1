cmake_minimum_required(VERSION 3.6)
project(ex1b)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES ex1b.c)
add_executable(ex1b ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(ex1b pthread)