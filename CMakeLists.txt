cmake_minimum_required(VERSION 3.10)
project(BlipKit)

set(CMAKE_C_FLAGS "-Wall -O2 -Wno-shift-negative-value")

add_subdirectory(src)
add_subdirectory(examples)
