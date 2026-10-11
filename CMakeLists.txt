cmake_minimum_required(VERSION 3.10)
set(CMAKE_TOOLCHAIN_FILE "$ENV{PSPDEV}/psp/share/pspdev.cmake" CACHE PATH "toolchain file")
project(me-mist C ASM)

set(MIST_SOURCES
  me-mist-kcall.c
  me-mist-utils.c
  me-mist.c
  me-mist-eshot.c
)

if(NOT PRX_FREE)
  add_subdirectory(kernel)

  add_custom_command(
    OUTPUT kcall.S
    COMMAND psp-build-exports -s ${CMAKE_SOURCE_DIR}/kernel/exports.exp > kcall.S
    DEPENDS kcall_prx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating kcall imports"
  )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/embedded.c
    COMMAND xxd -i ${CMAKE_CURRENT_BINARY_DIR}/kernel/kcall.prx > ${CMAKE_CURRENT_BINARY_DIR}/embedded.c
    COMMAND sed -i 's/unsigned char [^[]*/unsigned char embedded_kcall/' ${CMAKE_CURRENT_BINARY_DIR}/embedded.c
    COMMAND sed -i 's/unsigned int [^ ]*_len/unsigned int embedded_kcall_len/' ${CMAKE_CURRENT_BINARY_DIR}/embedded.c
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel/kcall.prx
    DEPENDS kcall_prx
    COMMENT "Embedding kcall.prx into C file"
  )

  add_custom_target(kcall_files DEPENDS kcall.S)

  list(APPEND MIST_SOURCES
    embedded.c
    ${CMAKE_CURRENT_BINARY_DIR}/kcall.S
  )
endif()

add_library(me-mist STATIC ${MIST_SOURCES})

if(NOT PRX_FREE)
  add_dependencies(me-mist kcall_files)
endif()

target_compile_options(me-mist PRIVATE 
  -Os
  -G0
  -Wall 
  -fno-pic
)

if(PRX_FREE)
  message(STATUS "PRX_FREE enabled")
  target_compile_definitions(me-mist PRIVATE PRX_FREE)
else()
  message(STATUS "PRX_FREE disabled")
endif()

target_include_directories(me-mist PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS me-mist
  ARCHIVE DESTINATION "$ENV{PSPDEV}/psp/lib"
)

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
list(FILTER HEADER_FILES EXCLUDE REGEX ".*\\.def\\.h$")

install(FILES ${HEADER_FILES}
  DESTINATION "$ENV{PSPDEV}/psp/include/me-mist"
)
