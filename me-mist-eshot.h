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
 
#ifndef ME_MIST_EASY_H
#define ME_MIST_EASY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EasyShot {
  void* task;
  unsigned int offset;
  unsigned int size;
} EasyShot;

extern void meEasyShot(const EasyShot shot);

#ifdef __cplusplus
}
#endif


#endif
