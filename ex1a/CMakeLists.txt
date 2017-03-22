cmake_minimum_required(VERSION 3.6)
project(ex1a)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        ex1a.c)

add_executable(ex1a ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(ex1a pthread)