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
 
#ifndef ME_MIST_MIST_H
#define ME_MIST_MIST_H

#include <me-core-mapper/me-core-mapper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MistInjector {
  
  int index;
  u32 addr;
} MistInjector;

typedef struct MistTrigger {
  
  int index;
  void* param;
} MistTrigger;

int  meMistInit();
void meMistInjectSyscall (MistInjector* const injector);
int  meMistTrigger(MistTrigger* const trigger);
void meMistWait();

int  meKernelMistInit();
void meKernelMistInjectSyscall(MistInjector* const injector);
int  meKernelMistTrigger(MistTrigger* const trigger);
void meKernelMistWait();

int  meMistFinish();

#ifdef __cplusplus
}
#endif


#endif

