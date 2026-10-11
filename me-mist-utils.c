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

#include "me-mist-utils.h"

void meMistIcacheInvalidateAll() {
  meMistSync();
  for (int i = 0; i < 8192; i += 64) {
    asm volatile(
        ".set push         \n"
        ".set noreorder    \n"
        "cache 0x04, 0(%0) \n"
        "cache 0x04, 0(%0) \n"
        ".set pop         \n"
        :: "r"(i)
        : "memory"
    );
  }
  meMistSync();
}

void meMistMemcpy (void* const dst, const void* const src, const unsigned int size) {
    
  unsigned int words = size >> 2;
  unsigned int rem   = size & 3;
  unsigned int* d32 = (unsigned int*)dst;
  const unsigned int* s32 = (const unsigned int*)src;
  while (words--) {
    *d32++ = *s32++;
  }
  unsigned char* d8 = (unsigned char*)d32;
  const unsigned char* s8 = (const unsigned char*)s32;
  while (rem--) {
    *d8++ = *s8++;
  }
}

void* meMistMemset (void* const dst, const int val, unsigned int size) {
  
  unsigned char *d8 = (unsigned char*)dst;
  while (size--) {
    *d8++ = (unsigned char)val;
  }
  return dst;
}

Aligned64 meMistGetAligned64Mem(char* const name, int mp, const unsigned int size) {
  
  const int ALIGNMENT = 64;
  SceUID uid = sceKernelAllocPartitionMemory(mp, name, PSP_SMEM_Low, size + ALIGNMENT, NULL);
  u32* mem = (u32*)((((ALIGNMENT - 1) + (unsigned int)sceKernelGetBlockHeadAddr(uid)) & ~(ALIGNMENT - 1)));
  
  return (Aligned64) {
    mem, uid
  };
}
