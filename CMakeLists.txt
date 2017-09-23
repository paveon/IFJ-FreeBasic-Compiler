cmake_minimum_required(VERSION 3.8)
project(freebasic_compiler)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c avl.c)
add_executable(freebasic_compiler ${SOURCE_FILES})