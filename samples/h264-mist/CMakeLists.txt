cmake_minimum_required(VERSION 3.10)
set(CMAKE_TOOLCHAIN_FILE "$ENV{PSPDEV}/psp/share/pspdev.cmake" CACHE PATH "toolchain file")
project(me-mist-h264 C CXX ASM)

add_executable(me-mist-h264
  main.c
)

target_compile_options(me-mist-h264 PRIVATE -Os -G0 -Wall -fno-pic)

target_compile_options(me-mist-h264 PRIVATE 
  $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
)

target_link_libraries(me-mist-h264
  me-core-mapper me-mist
  psppower pspdisplay pspctrl pspdebug
  pspge pspmpeg pspmpegbase
  pspkubridge
)

create_pbp_file(TARGET me-mist-h264 TITLE "MIST + H.264" BUILD_PRX)

