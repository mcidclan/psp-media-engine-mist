cmake_minimum_required(VERSION 3.10)
set(CMAKE_TOOLCHAIN_FILE "$ENV{PSPDEV}/psp/share/pspdev.cmake" CACHE PATH "toolchain file")
project(vme-mist-poc C CXX ASM)

add_executable(vme-mist-poc
  main.cpp
)

target_compile_options(vme-mist-poc PRIVATE -Os -G0 -Wall -fno-pic)

target_compile_options(vme-mist-poc PRIVATE 
  $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
)

target_link_libraries(vme-mist-poc
  me-core-mapper vme-lib me-mist
  psppower pspge pspdisplay pspctrl pspdebug pspkubridge
)

create_pbp_file(TARGET vme-mist-poc TITLE "VME DSP PoC" BUILD_PRX)

