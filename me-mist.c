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
 
#include <me-core-mapper/dmacplus.h>
#include "me-mist-utils.h"
#include "me-mist-kcall.h"
#include "me-mist.h"

#define CUSTOM_MAGIC_CHECK    0xC0FFEE
#define SYSCALL_PARAMS_BASE   0xbfc00600

//#define ME_PROCESSING         (*_ME_SHARED_MEM)
//#define ME_CUSTOM_PROCESS     (1 << 1)

//extern int  _ME_SHARED_MEM[16];
//extern int  meMistSelectTable();
//extern int  meKernelSelectTable();
//extern void meKernelTriggerCustomProcess();

static u32 SYSCALL_TABLE_ADDR = 0;
//static u32 SYSCALL_HANDLER_JUMP_ADDR = 0;
#define SYSCALL_HANDLER_JUMP_ADDR 0x883004c0

static int mistSelectTable(void* const param) {
  
  const int tableId = meCoreGetTableIdFromWitnessWord();
  if (tableId < 2) {
    return -1;
  }
  meCoreSelectSystemTable(tableId);
  return tableId;
}

static int meKernelMistSelectTable() {
  
  return mistSelectTable(NULL);
}

static int meMistSelectTable() {
  
  return meMistCall(mistSelectTable, NULL);
}

static int mistRefreshMe (void* param) {

  //int state = sceKernelSuspendDispatchThread();
  int intr = sceKernelCpuSuspendIntr();

  {
    // todo: to be tested on phat with 0x828e0, 0x824dc
    hw(0xbfc0071c) = 0x71e6c;
  }
  
  hw(0xbfc00704) = 0x1f;
  meMistSync();
  
  hw(0xbfc00040 + 0x20) = 0x20090003;
  hw(0xbfc00040 + 0x24) = 0x3c048840;
  hw(0xbfc00040 + 0x28) = 0;
  meMistSync();
  
  //hw(0xBC100070) = 3;
  //meMistSync();

  hw(0xbc10004c) |= 0x34;
  hw(0xbc10004c) = 0x0;
  meMistSync();
  
  sceKernelCpuResumeIntrWithSync(intr);
  //sceKernelResumeDispatchThread(state);

  return 0;
}

static int mistInjectSyscall (void* const param) {
    
  MistInjector* injector = (MistInjector*)param;
  const u32 tableBase = SYSCALL_TABLE_ADDR + (injector->index * 4);

  Aligned64 aligned = meMistGetAligned64Mem("mist-data-00", PSP_MEMORY_PARTITION_USER, 64);
  aligned.mem[0] = injector->addr;
  sceKernelDcacheWritebackRange(aligned.mem, 64);
  
  int state = sceKernelSuspendDispatchThread();
  cleanChannels();
  dmacplusFromSc((u32)aligned.mem, tableBase, DMACPLUS_BYTE_COUNT_4, 1);
  waitChannels();
  sceKernelResumeDispatchThread(state);
    
  sceKernelFreePartitionMemory(aligned.uid);
  return 0;
}

static void meKernelMistSetCurrentTrigger(void* const trigger) {
    
  const MistTrigger* currentTrigger = (MistTrigger*)trigger;
  u32* param = (u32*)SYSCALL_PARAMS_BASE;
  
  param[0] = (u32)currentTrigger->index;
  param[1] = 0;
  param[2] = (u32)currentTrigger->index;
  param[3] = (u32)currentTrigger->param;
  param[4] = CUSTOM_MAGIC_CHECK;
  param[5] = 0;
  param[6] = 0;
  param[7] = 0;
  param[8] = 0;
  param[9] = 0;
  
  //int intr = sceKernelCpuSuspendIntr();
  //ME_PROCESSING |= ME_CUSTOM_PROCESS;
  //sceKernelDcacheWritebackRange(&ME_PROCESSING, 64);
  //sceKernelCpuResumeIntrWithSync(intr);
  
  //sceKernelDcacheWritebackInvalidateRange(param, 40);
  //sceKernelIcacheInvalidateRange(param, 40);
}

static int init (const int table) {
  
  switch (table) {
    
    case ME_CORE_T2_IMG_TABLE:
      SYSCALL_TABLE_ADDR           = 0x003ce870;
      //SYSCALL_HANDLER_JUMP_ADDR    = 0x883004c0;
      break;
    case ME_CORE_IMG_TABLE:
      SYSCALL_TABLE_ADDR           = 0x0018a210;
      //SYSCALL_HANDLER_JUMP_ADDR    = 0x883004c0;
      break;
    case ME_CORE_BL_IMG_TABLE:
      break;
    case ME_CORE_SD_IMG_TABLE:
      break;
    default:
      return -4;
  }
  return 0;
}

static inline void triggerSysCall(const u32 index) {

  hw(0xbfc00700) = 0;
  meMistSync();
  hw(SYSCALL_PARAMS_BASE) = index;
  meMistDdrFlush(5);
  meMistEmitSoftInterrupt();
  //meMistDdrFlush(8);
}

void meKernelMistTriggerCustomProcess() {
  
  const u32 index = hw(SYSCALL_PARAMS_BASE);
  triggerSysCall(index);
}

static int mistTriggerTask (void* const trigger) {
  
  meKernelMistSetCurrentTrigger(trigger);
  meKernelMistTriggerCustomProcess();
  return 0;
}


// Refresh
static void meMistRefreshMe () {
  
  meMistCall(mistRefreshMe, NULL/*CACHED_KERNEL_MASK*/);
}

static void meKernelMistRefreshMe() {
  
  mistRefreshMe(NULL);
}

// Init
int meMistInit () {
  
  const int table = meMistSelectTable();
  int error = init(table);
  if (error) {
    return error;
  }
  meMistRefreshMe();
  return 0;
}

int meKernelMistInit () {
  
  const int table = meKernelMistSelectTable();
  int error = init(table);
  if (error) {
    return error;
  }
  meKernelMistRefreshMe();
  return 0;
}

// Inject
void meMistInjectSyscall (MistInjector* const injector) {
  
  kCall(mistInjectSyscall, CACHED_KERNEL_MASK, (void*)injector);
}

void meKernelMistInjectSyscall(MistInjector* const injector) {
  
  mistInjectSyscall((void*)injector);
}

// Trigger
int meMistTrigger (MistTrigger* const trigger) {
  
  const int ret = kCall(mistTriggerTask, CACHED_KERNEL_MASK, (void*)trigger);
  return ret;
}

int meKernelMistTrigger(MistTrigger* const trigger) {
  
  return mistTriggerTask((void*)trigger);
}

// Finish
int meMistFinish() {
  
  /*
  const u32 intr = meCoreInterruptClearMask();
  ME_PROCESSING &= ~ME_CUSTOM_PROCESS;
  meCoreDcacheWritebackRange((void*)_ME_SHARED_MEM, 64);
  meCoreInterruptSetMask(intr);
  */
  
  //hw(0xbfc00700) = -2;
  //meLibSync();

  asm volatile(
  
    ".set push;"
    ".set noreorder;"
    ".set volatile;"
    ".set noat;"
    "li $t0, 0xbfc00700;"
    "li $t1, -567;"
    "jr %0;"
    "sw $t1, 0($t0);"
    ".set pop;"
    :
    : "r" (SYSCALL_HANDLER_JUMP_ADDR)
    : "$t0", "$t1", "memory"
  );
  
  return 0;
}

static int mistWait() {

  int out = 0;
  while (1) {
    
    const int status = hw(0xbfc00700);
    meMistSync();
    
    if ((status == -567) || (++out > 500)) {
      break;
    }
    sceKernelDelayThread(1000);
  }
  
  return 0;
}

void meKernelMistWait() {
  
  mistWait();
}

void meMistWait() {
  
  kCall(mistWait, CACHED_KERNEL_MASK);
}
