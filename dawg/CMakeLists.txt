cmake_minimum_required(VERSION 3.31)
project(DAWG C)

set(CMAKE_C_STANDARD 99)

add_executable(DAWG main.c
        colors.c
        colors.h)
