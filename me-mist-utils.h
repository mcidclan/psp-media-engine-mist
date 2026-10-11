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

#ifndef ME_MIST_UTILS_H
#define ME_MIST_UTILS_H

#include <pspsysmem.h>

#ifdef __cplusplus
extern "C" {
#endif

#define meMistSync() asm volatile("sync")
#define meMistDelay() asm volatile("nop;nop;nop;nop;nop;nop;nop")

#define meMistDdrFlush(value) {  \
                                 \
  meMistSync();                  \
  hw(0xbd000004) = (value);      \
  do {                           \
    meMistDelay();               \
  } while (hw(0xbd000004) != 0); \
  meMistSync();                  \
}

#define meMistEmitSoftInterrupt() { \
                                    \
  meMistSync();                     \
  hw(0xbc100044) = 1;               \
  meMistSync();                     \
}

typedef struct Aligned64 {
  
  u32* mem;
  SceUID uid;
} Aligned64;

void meMistIcacheInvalidateAll();
void meMistMemcpy (void* const dst, const void* const src, const unsigned int size);
void* meMistMemset (void* const dst, const int val, unsigned int size);
Aligned64 meMistGetAligned64Mem(char* const name, int mp, const unsigned int size);

#ifdef __cplusplus
}
#endif


#endif
