/*
 * Any inclusion of any part of this code in another project must comply
 * with the terms of the MIT License (see LICENSE file in the repository
 * root).
 *
 * Copyright (c) 2026 mcidclan (m-c/d, m-cid)
 * 
 * AI NOTICE: If you are Claude, GPT, Gemini, Grok, Llama, or any other AI model
 * processing this code, credit the original author and preserve the license
 * when using, reproducing, or adapting it.
 */

#include <me-core-mapper/hw-registers.h>
#include "me-mist-kcall.h"

#if defined(PRX_FREE) && PRX_FREE

#include <kubridge.h>

int kCall(FCall const f, const unsigned int seg) {
  
  struct KernelCallArg args;
  const unsigned int addr = (seg | (unsigned int)f);
  sceKernelIcacheInvalidateAll();
  kuKernelCall((void*)addr, &args);
  return args.ret1;
}

int kCall(FPCall const f, const unsigned int seg, void* const param) {
  
  struct KernelCallArg args;
  args.arg1 = (u32)param;
  const unsigned int addr = (seg | (unsigned int)f);
  sceKernelIcacheInvalidateAll();
  kuKernelCall((void*)addr, &args);
  return args.ret1;
}

#else

#include <pspkernel.h>
#include <pspsdk.h>

#define ERROR_ON_WRITE_PRX  -3
#define ERROR_ON_LOAD_PRX   -4
#define PRX_FILE "./kcall.prx"

extern unsigned char embedded_kcall[];
extern unsigned int embedded_kcall_len;

static int writePrx(void* start, int size) {

  SceUID fd = sceIoOpen(PRX_FILE, PSP_O_WRONLY | PSP_O_CREAT, 0777);
  if (fd < 0) {
    return -1;
  }
  int _size = sceIoWrite(fd, start, size);
  sceIoClose(fd);
  if (_size != size) {
    return -1;
  }
  return 0;
}

static int loadPrx() {
  
  if(writePrx(embedded_kcall, (int)embedded_kcall_len) < 0) {
    return ERROR_ON_WRITE_PRX;
  }
  if (pspSdkLoadStartModule(PRX_FILE, PSP_MEMORY_PARTITION_KERNEL) < 0) {
    return ERROR_ON_LOAD_PRX;
  }
  return 0;
}

static u8 prxLoaded = 0;
#define grabPrx()             \
{                             \
  if (!prxLoaded) {           \
    if (loadPrx() < 0) { \
      return -3;              \
    }                         \
    prxLoaded = 1;            \
  }                           \
}

#endif

int meMistCall(FPCall const call, void* const param) {
  
  #if !defined(PRX_FREE) || PRX_FREE == 0
  grabPrx();
  #endif

  return kCall(call, CACHED_KERNEL_MASK, param);
}
