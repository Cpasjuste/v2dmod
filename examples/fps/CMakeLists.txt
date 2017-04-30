#set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 2.8)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if (DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else ()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif ()
endif ()

project(v2d_fps)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")

include_directories(
        ${CMAKE_SOURCE_DIR}/v2dmod
)
link_directories(
        ${CMAKE_BINARY_DIR}/v2dmod/libv2dmod
        ${CMAKE_CURRENT_BINARY_DIR})

add_executable(v2d_fps main.c)
add_dependencies(v2d_fps libv2dmod)

target_link_libraries(v2d_fps
        taihen_stub
        v2dmod_stub
        SceLibKernel_stub
        k gcc
        )

vita_create_self(v2d_fps.suprx v2d_fps
        CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/v2d_fps.yml
        )