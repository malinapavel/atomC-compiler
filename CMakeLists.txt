cmake_minimum_required(VERSION 3.24)
project(LabCT C)

set(CMAKE_C_STANDARD 11)

add_executable(LabCT main.c library_compiler.h lexical.c syntactical.c)
