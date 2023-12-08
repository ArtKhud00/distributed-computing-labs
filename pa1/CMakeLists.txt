cmake_minimum_required(VERSION 3.16)
project(lab1 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_C_FLAGS "-std=c99 -Wall -pedantic -Werror")

add_executable(lab1 main.c ipc.h pa1.h common.h constants.h pipe_operations.h pipe_operations.c ipc.c child_operations.c child_operations.h parent_operations.c parent_operations.h general_actions.c general_actions.h)