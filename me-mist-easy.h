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
 
#include "me-mist-kcall.h"
#include "me-mist.h"

#define MIST_SYSCALL_INDEX_13 13

#define MIST_TASK(name, index, param, ...) \
int name(int index, void* param) { \
  if (param) { __VA_ARGS__ } \
  return meMistFinish(); \
}

static inline int meEasyMistRegisterTrigger(MistTrigger* const trigger, const u32 task) {

  static int init = 0;
  
  if (!init) {
    
    init = -1;
    
    const int error = meMistInit();
    if (error >= 0) {
      init = 1;
    }
  }

  if (init > 0) {
    
    MistInjector injector = {
      trigger->index, CACHED_KERNEL_MASK | task
    };
    meMistInjectSyscall(&injector);
    return 0;
  }
  return -1;
}

static inline void meEasyMistTrigger(MistTrigger* const trigger) {
  
  meMistTrigger(trigger);
}

static inline void meEasyMistTriggerAndWait(MistTrigger* const trigger) {
  
  meMistTrigger(trigger);
  meMistWait();
}
