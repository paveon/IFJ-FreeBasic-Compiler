cmake_minimum_required(VERSION 3.8)
project(freebasic_compiler)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -O0 -std=c99")

set(SOURCE_FILES main.c Token.h Token.c CompilationErrors.h)
add_executable(freebasic_compiler ${SOURCE_FILES})