cmake_minimum_required(VERSION 3.0)
project(bmf2c)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp BMFont.cpp BMFont.h)
add_executable(bmf2c ${SOURCE_FILES})

