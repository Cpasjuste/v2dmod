#set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 2.8)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if (DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else ()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif ()
endif ()

project(v2dmod)
include("${VITASDK}/share/vita.cmake" REQUIRED)
enable_language(ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99 -D__MODULE__")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions -D__MODULE__")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")

include_directories(
        libvita2d/libvita2d/include
)

link_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
)

if (NOT ${RELEASE})
    add_definitions(-DENABLE_LOGGING)
endif ()

file(GLOB SRC
        libvita2d/libvita2d/shader/compiled/color_f_gxp.o
        libvita2d/libvita2d/shader/compiled/color_v_gxp.o
        libvita2d/libvita2d/shader/compiled/texture_f_gxp.o
        libvita2d/libvita2d/shader/compiled/texture_v_gxp.o
        libvita2d/libvita2d/shader/compiled/texture_tint_f_gxp.o
        )

add_executable(v2dmod
        main.c
        v2dmod.c
        v2dmod_fb.c
        v2dmod_fb_font.c
        v2dmod_drawing.c
        v2dmod_utility.c
        v2dmod_log.c
        libvita2d/libvita2d/source/vita2d.c
        libvita2d/libvita2d/source/vita2d_draw.c
        libvita2d/libvita2d/source/vita2d_texture.c
        libvita2d/libvita2d/source/vita2d_image_bmp.c
        libvita2d/libvita2d/source/utils.c
        libvita2d/libvita2d/source/int_htab.c
        libvita2d/libvita2d/source/texture_atlas.c
        libvita2d/libvita2d/source/bin_packing_2d.c
        libvita2d/libvita2d/source/vita2d_pgf.c
        ${SRC}
        )

target_link_libraries(v2dmod
        taihen_stub
        SceLibKernel_stub
        SceIofilemgr_stub
        SceDisplay_stub
        SceSysmem_stub
        SceGxm_stub
        ScePgf_stub
        SceSysmodule_stub
        ScePower_stub
        SceKernelThreadMgr_stub
        SceKernelModulemgr_stub
        SceAppMgr_stub
        kuio_stub
        k
        gcc
        m
        )

vita_create_self(v2dmod.suprx v2dmod
        CONFIG ${CMAKE_SOURCE_DIR}/v2dmod/v2dmod.yml)

# fix "make" command not finding VITASDK env variable
# on my system..
macro(_vita_create_stubs target-dir source config)
    set(VITA_ELF_EXPORT_FLAGS "${VITA_ELF_EXPORT_FLAGS}" CACHE STRING "vita-elf-export flags")
    set(VITA_LIBS_GEN_FLAGS "${VITA_LIBS_GEN_FLAGS}" CACHE STRING "vita-libs-gen flags")

    set(options KERNEL)
    cmake_parse_arguments(_vita_create_stubs "${options}" "" "" ${ARGN})

    if (_vita_create_stubs_KERNEL)
        set(kind kernel)
    else ()
        set(kind user)
    endif ()
    separate_arguments(VITA_ELF_EXPORT_FLAGS)
    get_filename_component(fconfig ${config} ABSOLUTE)
    MESSAGE(info OUTPUT ${target-dir}.yml
            COMMAND ${VITA_ELF_EXPORT} ${kind} ${VITA_ELF_EXPORT_FLAGS} ${source} ${fconfig} ${target-dir}.yml
            DEPENDS ${source} ${fconfig}
            COMMENT "Generating imports YAML for ${source}")
    add_custom_command(OUTPUT ${target-dir}.yml
            COMMAND ${VITA_ELF_EXPORT} ${kind} ${VITA_ELF_EXPORT_FLAGS} ${source} ${fconfig} ${target-dir}.yml
            DEPENDS ${source} ${fconfig}
            COMMENT "Generating imports YAML for ${source}"
            )
    separate_arguments(VITA_LIBS_GEN_FLAGS)
    add_custom_target(${target-dir} ALL
            COMMAND ${VITA_LIBS_GEN} ${VITA_LIBS_GEN_FLAGS} ${target-dir}.yml ${target-dir}
            # modified
            COMMAND VITASDK=${VITASDK} make -C ${target-dir}
            # modified
            DEPENDS ${target-dir}.yml
            COMMENT "Building stubs ${target-dir}"
            )
    add_dependencies(${target-dir} ${source})
endmacro(_vita_create_stubs)

_vita_create_stubs(libv2dmod v2dmod
        ${CMAKE_SOURCE_DIR}/v2dmod/v2dmod.yml)
add_dependencies(libv2dmod v2dmod)

install(DIRECTORY ${CMAKE_BINARY_DIR}/libv2dmod/
        DESTINATION lib
        FILES_MATCHING PATTERN "*.a"
        )

install(FILES v2dmod.h v2dmod_drawing.h
        DESTINATION include
        )
