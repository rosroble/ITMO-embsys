cmake_minimum_required(VERSION 3.19)
project(embedded_systems C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(peparser main.c)