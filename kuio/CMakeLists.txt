cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(kuio)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DRELEASE)

add_executable(kuio main.c kpool.c)

target_link_libraries(kuio
  taihenForKernel_stub
  SceLibKernel_stub
  SceSysmemForDriver_stub
  SceThreadmgrForDriver_stub
  SceIofilemgrForDriver_stub
  k
  gcc
)

set_target_properties(kuio
  PROPERTIES LINK_FLAGS "-nostdlib"
  COMPILE_FLAGS "-D__VITA_KERNEL__"
)

vita_create_self(kuio.skprx kuio CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/kuio.yml)

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

_vita_create_stubs(kuio_stubs kuio ${CMAKE_CURRENT_SOURCE_DIR}/kuio.yml KERNEL)
