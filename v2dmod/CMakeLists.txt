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
        vita2d
        ${CMAKE_SOURCE_DIR}/kuio/include
)

link_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_BINARY_DIR}/kuio/kuio_stubs
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_DEBUG true CACHE BOOL "Debug build")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DENABLE_LOGGING)
endif ()

add_executable(v2dmod
        main.c
        v2dmod.c
        v2dmod_drawing.c
        v2dmod_utility.c
        v2dmod_log.c
        v2dmod_config.c
        v2dmod_kpool.c
        menu/menu_main.c
        menu/menu_module_option.c
        menu/menu_drawing_helper.c
        data/impact-23-outline.fnt.c
        vita2d/vita2d.c
        vita2d/vita2d_draw.c
        vita2d/vita2d_texture.c
        vita2d/vita2d_image_bmp.c
        vita2d/vita2d_bmf.c
        vita2d/utils.c
        vita2d/shader/compiled/color_f_gxp.o
        vita2d/shader/compiled/color_v_gxp.o
        vita2d/shader/compiled/texture_f_gxp.o
        vita2d/shader/compiled/texture_v_gxp.o
        vita2d/shader/compiled/texture_tint_f_gxp.o
        menu/menu_module_list.c menu/menu_module_list.h)

set(LDFLAGS
        taihen_stub
        SceLibKernel_stub
        SceIofilemgr_stub
        SceDisplay_stub
        SceSysmem_stub
        SceGxm_stub
        SceKernelThreadMgr_stub
        SceKernelModulemgr_stub
        SceAppMgr_stub
        kuio_stub
        k
        gcc
        m)

target_link_libraries(v2dmod ${LDFLAGS})

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
