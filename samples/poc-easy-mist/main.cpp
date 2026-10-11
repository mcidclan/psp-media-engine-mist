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
 
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psppower.h>
#include <me-mist/me-mist-easy.h>

PSP_MODULE_INFO("mist-demo-easy", 0, 1, 1);
PSP_HEAP_SIZE_KB(-1024);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU | PSP_THREAD_ATTR_USER);

u32 meCounter[16] __attribute__((aligned(64))) = {0};

MistTrigger trigger = {
  MIST_SYSCALL_INDEX_13, meCounter
};

MIST_TASK(mistTask, index, param, {
  
  meCoreDcacheInvalidateRange(param, 64);
  u32* const meCounter = (u32*)param;
  *meCounter += 1;
  meCoreDcacheWritebackRange(param, 64);
});

int meThread(SceSize args, void *argp) {
  
  int* const meStopped = (int*)*((int*)argp);
  while (!*meStopped) {
    
    meEasyMistTriggerAndWait(&trigger);
    sceKernelDcacheWritebackInvalidateAll();
    sceKernelDelayThread(1);
  }
  
  *meStopped = 1;
  return sceKernelExitDeleteThread(0);
}

int main() {
  
  scePowerSetClockFrequency(333, 333, 166);
  pspDebugScreenInit();
  
  int thid = -1;
  int meStopped = 0;
  
  const int error = meEasyMistRegisterTrigger(&trigger, (u32)mistTask);
  if (error >= 0) {
    
    thid = sceKernelCreateThread("me-thread", meThread, 0x18, 0x2000, PSP_THREAD_ATTR_VFPU, 0);
    if (thid >= 0) {
      int* param[1] = { &meStopped };
      sceKernelStartThread(thid, sizeof(meStopped), &param);
    }
  }
  else {
    meStopped = 1;
  }
  
  SceCtrlData ctl;
  u32 counter = 0;
  do {

    sceCtrlPeekBufferPositive(&ctl, 1);
    pspDebugScreenSetXY(0, 0);
    pspDebugScreenPrintf("meCounter: 0x%08lx", *meCounter);
    pspDebugScreenSetXY(0, 1);
    pspDebugScreenPrintf("scCounter: 0x%08lx", counter++);
    
    sceDisplayWaitVblank();
    sceKernelDelayThread(1);
  
  } while (!(ctl.Buttons & PSP_CTRL_HOME) || meStopped);
  
  if (meStopped) {
    
    pspDebugScreenSetXY(0, 2);
    pspDebugScreenPrintf("ME thread not running, exiting...");
  }
  
  if (thid >= 0) {
    
    SceUInt timeout = 500000;
    sceKernelWaitThreadEnd(thid, &timeout);
    sceKernelTerminateDeleteThread(thid);
  }
  sceKernelExitGame();
  return 0;
}


