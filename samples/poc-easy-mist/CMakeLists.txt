cmake_minimum_required(VERSION 3.10)
set(CMAKE_TOOLCHAIN_FILE "$ENV{PSPDEV}/psp/share/pspdev.cmake" CACHE PATH "toolchain file")
project(me-mist-easy-mist C CXX ASM)

add_executable(me-mist-easy-mist
  main.cpp
)

target_compile_options(me-mist-easy-mist PRIVATE -Os -G0 -Wall -fno-pic)

target_compile_options(me-mist-easy-mist PRIVATE 
  $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
)

target_link_libraries(me-mist-easy-mist
  me-core-mapper me-mist
  psppower pspge pspdisplay pspctrl pspdebug
  pspkubridge
)

create_pbp_file(TARGET me-mist-easy-mist TITLE "Me MIST Easy PoC" BUILD_PRX)

