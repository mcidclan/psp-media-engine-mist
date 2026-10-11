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
 
#include "me-mist-eshot.h"
#include "me-mist-utils.h"

#include "me-mist-kcall.h"
#include <me-core-mapper/hw-registers.h>
#include <pspkernel.h>

static int kernelEasyShot(void* _shot) {
  
  if (!_shot) {
    return -1;
  }
  
  EasyShot* const shot = (EasyShot*)_shot;
  if (!shot->size) {
    return -1;
  }
  
  meMistMemcpy((void*)(0x88300000 + shot->offset), shot->task, shot->size);

  hw(0xbfc00040 + 0x20) = 0x20090003;
  hw(0xbfc00040 + 0x24) = 0x3c048840;
  hw(0xbfc00040 + 0x28) = 0;
  meMistSync();

  sceKernelDcacheWritebackInvalidateAll();
  sceKernelIcacheInvalidateAll();

  hw(0xbc10004c) |= 0x1434; // 0x14;
  hw(0xbc10004c) = 0x0;
  meMistSync();
  
  hw(0xBC100070) |= 0x05;
  meMistSync();

  return 0;
}

void meEasyShot(const EasyShot shot) {
  meMistCall(kernelEasyShot, (void*)&shot);
}
