#ifndef H264_MAIN_HEADER
#define H264_MAIN_HEADER

#include <pspkernel.h>
#include <pspdisplay.h>
#include <psppower.h>
#include <pspmpeg.h>
#include <pspmpegbase.h>
#include <psputility_avmodules.h>
#include <pspctrl.h>

#include <string.h>
#include <malloc.h>

#include <me-mist/me-mist-easy.h>
#include <me-core-mapper/me-core-mapper.h>

struct DmacPlusLLI {
  
  void* src;
  void* dst;
  void* next;
  unsigned int count;
}; // __attribute__((aligned(4)));

struct MpegDMAC {
  
  struct DmacPlusLLI* lli;
  void* src;
  void* dst;
  int count;
  int size;
};

struct H264 {
  
  void* frame;
  void* mpeg;
  unsigned int* data;
  struct MpegDMAC dmac;
  SceMpegAu accessUnit;
};

struct RGB {

  int format;
  int stride;
  int height;
  int bpp;
  
  char pad[48];
  volatile void* frame __attribute__((aligned(64)));
} __attribute__((aligned(64)));

#endif
