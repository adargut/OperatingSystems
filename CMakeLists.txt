cmake_minimum_required(VERSION 3.15)
project(OSClass C)

set(CMAKE_C_STANDARD 90)

add_executable(OSClass Ex1/os.c Ex1/os.h Ex2/shell.c)