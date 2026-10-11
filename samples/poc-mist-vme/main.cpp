
/*
 * VME + MIST PoC
 * 
 * First sample code using the PSP DSP known as VME2. It is a POC demonstrating
 * the VME flow and how to perform an operation on a set of 32 values through a
 * 3-stage fixed-point Q.23 pipeline that multiplies each value of a buffer
 * by a variable factor.
 *
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
 
#include "main.h"

PSP_MODULE_INFO("vme-dsp-dump", 0, 1, 1);
PSP_HEAP_SIZE_KB(-1024);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU | PSP_THREAD_ATTR_USER);

#define ME_SYSCALL_INDEX_0 20
#define ME_SYSCALL_INDEX_1 21

#define DATA_COUNT 64
#define BYTE_COUNT (DATA_COUNT * 4)
#define DEFAULT_FACTOR 0.5f

float factor = DEFAULT_FACTOR;

u32 meDataGen[DATA_COUNT] __attribute__((aligned(64))) = {0};
u32 meDataOut[DATA_COUNT] __attribute__((aligned(64))) = {0};
u32 meVars[16] __attribute__((aligned(64))) = {0};

static_assert(sizeof(meVars) == 64, "meVars size must be 64-byte aligned");

const float values[] __attribute__((aligned(64))) = {
  0.12f,  0.35f,  0.67f,  0.89f,
  0.45f,  0.23f,  0.78f,  0.56f,
  0.91f,  0.34f,  0.11f,  0.72f,
  0.48f,  0.63f,  0.82f,  0.19f,
 -0.25f, -0.47f, -0.63f, -0.81f,
 -0.12f, -0.55f, -0.38f, -0.74f,
 -0.92f, -0.17f, -0.44f, -0.69f,
 -0.31f, -0.58f, -0.83f, -0.96f,
};

static_assert(sizeof(values) % 64 == 0, "values size must be 64-byte aligned");

int meInit(int index, void* param) {

  if (param) {
    
    int size = sizeof(values);
    meCoreDcacheInvalidateRange(param, size);
    
    vmeLibEnable();
    vmeLibWipe();

    float* const input = (float*)param;
    u32* const top = (u32*)VME_TOP_BUFFERS;
    size = size / sizeof(values[0]);
    
    for (int i = 0; i < size; i++) {
      top[i] = F2Q(input[i]);
    }

    // Start of the VME related code
    vmeLibStart();
    
    vme_icn(FLOW, 0);
    vme_icn(ARCH, VME_DEF_MAPPER);

    const u8  k = Q_FORMAT;
    const u32 b = F2Q(DEFAULT_FACTOR);
    
    const int prologue = 0x10;
    const int count = (size + prologue) - 1;
  
    {
      // r1 = (x0 * b) >> k
      const u32 op = 0x00204000;
      const u32 mux = vme_mux(TOP_0);

      vme_pe0(vme_fu(PRIMARY), mux, op, k);
      vme_pe0(fu_reg(PRIMARY, B), b);
      
      // x0 source control
      vme_pe0(agu_top(MODE), VME_DEF_MODE);
      vme_pe0(agu_top(COUNT), VME_DEF_STEP, count);
      
      // r1 source control
      vme_pe0(agu_base(MODE), VME_DEF_MODE);
      vme_pe0(agu_base(COUNT), VME_DEF_STEP, count);
      
      // r1 destination control
      vme_pe0(agu_write(MODE), VME_DEF_MODE, VME_CYCLE_6);
      vme_pe0(agu_write(COUNT), VME_DEF_STEP, count);

      // force update over local buffer with a 0x10 prologue/padding
      // necessary to get the correct result from the first cycle
      vme_pe0(agu_write(FORMAT_0), prologue);
      vme_pe0(agu_write(FORMAT_1), VME_END_TOKEN);
    }
    
    {
      // r2 = (r1 * b) >> k
      const u32 op = 0x00204000;
      const u32 mux = vme_mux(BASE_0, STAGING);
      
      vme_pe1(vme_fu(PRIMARY), mux, op, k);
      vme_pe1(fu_reg(PRIMARY, B), b);
      
      // r2 destination control
      vme_pe1(agu_write(MODE), VME_DEF_MODE, VME_CYCLE_9);
      vme_pe1(agu_write(COUNT), VME_DEF_STEP, count);
      
      // force update over local buffer with a 0x10 prologue/padding
      // necessary to get the correct result from the first cycle
      vme_pe1(agu_write(FORMAT_0), prologue);
      vme_pe1(agu_write(FORMAT_1), VME_END_TOKEN);
    }
    
    {
      // r3 = r1 + r2
      const u32 op = 0x00030000;
      const u32 mux = vme_mux(BASE_1, BASE_0);
      
      vme_pe2(vme_fu(PRIMARY), mux, op);
      
      // r3 destination control
      const int offset = 0x10000 - prologue; // cancel prologue/padding (-0x10)
      vme_pe2(agu_write(MODE), VME_DEF_MODE, VME_CYCLE_6, offset);
      vme_pe2(agu_write(COUNT), VME_DEF_STEP, count);
    }
    
    // End of the VME related code
    vmeLibFinish();
    vmeLibDisable();
  }
  
  return meMistFinish();
}

struct MeShared {
  
  u32* dataGen;
  u32* dataOut;
  u32* vars;
  u32 pad[13];
} __attribute__((aligned(64)));

static_assert(sizeof(MeShared) % 64 == 0, "MeShared size must be 64-byte aligned");

int meRun(int index, void* param) {
  
  if (param) {
    
    vmeLibEnable();

    meCoreDcacheInvalidateRange(param, sizeof(MeShared));
    const MeShared* const shared = (MeShared*)param;
    meCoreDcacheInvalidateRange(shared->vars, sizeof(meVars));
    
    // Start of the VME datapath update
    vmeLibStart();
    vme_pe0(fu_reg(PRIMARY, B), shared->vars[0]);
    vme_pe1(fu_reg(PRIMARY, B), shared->vars[1]);

    // End of the VME datapath update
    vmeLibFinish();
    
    u32* const dataGen = (u32*)shared->dataGen;
    u32* const dataOut = (u32*)shared->dataOut;
    
    if (!shared->vars[2]) {
      
      meCoreMemcpy(dataGen, (void*)(VME_TOP_BUFFERS), BYTE_COUNT);
      meCoreDcacheWritebackRange(dataGen, BYTE_COUNT);
      shared->vars[2] = 1;
    }
    
    meCoreMemcpy(dataOut, (void*)(VME_BASE_BUFFERS + 8192*2), BYTE_COUNT);
    meCoreDcacheWritebackRange(dataOut, BYTE_COUNT);
    meCoreDcacheWritebackRange(shared->vars, sizeof(meVars));

    vmeLibDisable();
  }
  return meMistFinish();
}

int meThread(SceSize args, void *argp) {

  {
    MistInjector injector = {
      ME_SYSCALL_INDEX_0, CACHED_KERNEL_MASK | (u32)meInit
    };
    meMistInjectSyscall(&injector);
    MistTrigger trigger = { ME_SYSCALL_INDEX_0, (void*)values };
    sceKernelDcacheWritebackRange(values, sizeof(values));
    
    meMistTrigger(&trigger);
    meMistWait();
  }
  
  {
    MistInjector injector = {
      ME_SYSCALL_INDEX_1, CACHED_KERNEL_MASK | (u32)meRun
    };
    meMistInjectSyscall(&injector);
  }

  const MeShared shared = {meDataGen, meDataOut, meVars};
  MistTrigger trigger = {
    ME_SYSCALL_INDEX_1, (void*)&shared
  };
  sceKernelDcacheWritebackRange(&shared, sizeof(shared));
  
  bool up = false;
  float lastFactor = 0.0f;

  SceCtrlData ctl;
  int* const meStopped = (int*)*((int*)argp);
  
  while (!*meStopped) {
    
    sceCtrlPeekBufferPositive(&ctl, 1);
    
    if (!(ctl.Buttons & PSP_CTRL_TRIANGLE) && !(ctl.Buttons & PSP_CTRL_CROSS)) {
      
      up = true;
    } else if (up) {
  
      if (ctl.Buttons & PSP_CTRL_TRIANGLE) {
          factor = (factor + 0.1f > 1.0f) ? 1.0f : factor + 0.1f;
          up = false;
      }
      else if (ctl.Buttons & PSP_CTRL_CROSS) {
          factor = (factor - 0.1f < 0.1f) ? 0.1f : factor - 0.1f;
          up = false;
      }
    }
    
    if (lastFactor != factor) {
      
      getB(factor, &meVars[0], &meVars[1]);
      sceKernelDcacheWritebackRange(meVars, sizeof(meVars));
      
      meMistTrigger(&trigger);
      meMistWait();
      
      sceKernelDcacheWritebackInvalidateRange(meDataGen, BYTE_COUNT);
      sceKernelDcacheWritebackInvalidateRange(meDataOut, BYTE_COUNT);
      sceKernelDcacheInvalidateRange(meVars, sizeof(meVars));

      lastFactor = factor;
    }
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
  
  const int error = meMistInit();
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
  do {
    
    sceCtrlPeekBufferPositive(&ctl, 1);
    
    const u32* const gen = meDataGen;
    const u32* const out = meDataOut;
    
    pspDebugScreenSetXY(0, 0);
    pspDebugScreenPrintf("ME Generated Data:");
    
    for(int i = 0; i < 8; i++) {
      
      const int off = i * 4;
      pspDebugScreenSetXY(0, i+2);
      pspDebugScreenPrintf("%f, %f, %f, %f,",
        Q2F(gen[off+0]), Q2F(gen[off+1]), Q2F(gen[off+2]), Q2F(gen[off+3]));
    }
    
    pspDebugScreenSetXY(0, 12);
    pspDebugScreenPrintf("VME Output Data:");
    for(int i = 0; i < 8; i++) {
      
      const int off = i * 4;
      pspDebugScreenSetXY(0, i+14);
      pspDebugScreenPrintf("%f, %f, %f, %f,",
        Q2F(out[off+0]), Q2F(out[off+1]), Q2F(out[off+2]), Q2F(out[off+3]));
    }

    pspDebugScreenSetXY(0, 24);
    pspDebugScreenPrintf("Input factor: %f", factor);
    pspDebugScreenSetXY(0, 25);
    pspDebugScreenPrintf("Use Triangle/Cross to change its values");
            
    sceDisplayWaitVblank();
    sceKernelDelayThread(1);
    
  } while (!(ctl.Buttons & PSP_CTRL_HOME) || meStopped);
  
  if (meStopped) {
    pspDebugScreenSetXY(0, 33);
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
