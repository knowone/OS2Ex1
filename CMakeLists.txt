cmake_minimum_required(VERSION 3.6)
project(ex1)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES ex1a.c)
add_executable(ex1 ${SOURCE_FILES})