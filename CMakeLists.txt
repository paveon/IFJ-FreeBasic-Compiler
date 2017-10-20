cmake_minimum_required(VERSION 3.8)
project(freebasic_compiler)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -O0 -std=c99")
include_directories(Source)

set(SOURCE_FILES
        Source/main.c Source/Token.c Source/CompilationErrors.c
        Source/symtable.c Source/Stack.c Source/LLtable.c Source/TopDown.c
        Source/Lexical.c Source/PrecedentTable.c Source/BottomUp.c)
set(HEADER_FILES
        Source/CompilationErrors.h Source/Token.h Source/symtable.h
        Source/Stack.h Source/LLtable.h Source/Symbol.h Source/TopDown.h
        Source/Lexical.h Source/PrecedentTable.h Source/BottomUp.h)
add_executable(freebasic_compiler ${SOURCE_FILES} ${HEADER_FILES})