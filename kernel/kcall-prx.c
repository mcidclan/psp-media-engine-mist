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
 
#include "../me-mist-kcall.h"
#include <pspsdk.h>
#include <psputilsforkernel.h>


PSP_MODULE_INFO("kcall", 0x1006, 1, 1);
PSP_NO_CREATE_MAIN_THREAD();

int kCall(FCall const f, const unsigned int seg) {
  const unsigned int addr = (seg | (unsigned int)f);
  sceKernelIcacheInvalidateAll();
  /*
  switch (seg) {
    case 1: return ((FCall)(0x80000000 | (unsigned int)f))();
    case 2: return ((FCall)(0x40000000 | (unsigned int)f))();
    case 3: return ((FCall)(0xa0000000 | (unsigned int)f))();
  }
  */
  return ((FCall)addr)();
}

int kCall(FPCall const f, const unsigned int seg, void* const param) {
  const unsigned int addr = (seg | (unsigned int)f);
  sceKernelIcacheInvalidateAll();
  return ((FPCall)addr)(param);
}

int module_start(SceSize args, void *argp) {
  return 0;
}

int module_stop() {
  return 0;
}
