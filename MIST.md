# PSP Media Engine

What we're doing today, will be the foundation of what will be done tomorrow,  
thank you to the pioneers and thank you to all who come after.  
I'm happy to share this with you.

## The ME MIST library and the MIST Method

The Media Engine MIST `me-mist` library uses new ways to trigger custom tasks on the Media Engine, with the goal of not disrupting Syscalls from the main CPU to the Media Engine. Before MIST, 2 other methods were first added to the library. They involve patching the Media Engine core, which is based in kernel memory at address `0x88300000`.


### Method 1: Syscall handler redirection

This first method patches the Media Engine Syscall handler, responsible for calling core functions using an entry index from the Syscall table. The idea is straightforward: redirect execution to a custom function whenever an invalid index is encountered.

That said, this patch alone isn't sufficient. It works in tandem with a second patch applied to a function loaded and launched from the main CPU, whose role is to trigger a cache update on the ME side. A third patch is also introduced over the interrupt handler, to distinguish custom tasks from system tasks.

The ideal candidate for that second patch was `sceAudiocodecGetEDRAM`. When called from the main CPU, it dispatches the corresponding Syscall to the ME through the me-wrapper, and this later triggers a soft interrupt to it.

On the ME side, this process invokes `meCoreGetMeEdram`, which was patched to perform an ICache invalidation, making sure the modified Syscall handler and interrupt handler are properly picked up by the ME instruction pipeline. Note that this method requires the `PSP_MODULE_AV_AVCODEC` module to be loaded.

It's also worth mentioning that the ME handler natively contains `cacheOp` operations that appear to ensure a DCache and ICache fill on the targeted Syscall function's address, though the exact address involved was not fully verified.

Working through this method helped lift the veil on several things that were still unclear to me, and gave me a much better understanding of the subject. It works, but since the second method turned out to be simpler, it made sense to move on.


### Method 2: Existing Syscall hijacking

This method takes a different approach: instead of redirecting invalid Syscall indices, it hijacks an existing Syscall that is simple enough to patch cleanly.

To do this, more information about the available Syscalls was needed, specifically the table containing their addresses. The first method described above was repurposed here as a spy tool: it was used to observe CPU registers just before a Syscall function call, and to confirm the base address of the table by fully dumping the 4MB of EDRAM (slim). That table is available [here](https://github.com/mcidclan/psp-media-engine-cracking-the-unknown/blob/main/media-engine-t2img.syscall-table.txt).

From this table, a function with very few instructions was selected as the target. It is involved in H.264 decoding, and is therefore never called by the system when launching a homebrew in game mode. In that context, calling it can only happen explicitly, either through a higher-level function related to the H.264 decoder, or by invoking its Syscall directly, which is an advantage offered by this method. Furthermore, its first call naturally triggers an ICache fill through the handler itself, which is convenient since only that one function needed to be patched.

That said, this approach has real limitations. It isn't viable in contexts where the Media Engine has potentially already performed decoding. More broadly, both of the previous Methods are problematic on the XMB, due to the hot modifications they require on the host kernel area of the core.


### Method 3: Media Engine Injected Syscall using Transfer (MIST)

This third method was born from a clear constraint: execute code on the Media Engine without writing to the reset vector, and without touching the kernel area that houses the core.

The insight came from the (possibly incomplete) Syscall mapping available: rather than patching existing code, what if a custom address could be injected directly into the Syscall table, pointing to a location outside of the core entirely?

The answer came from the DMACPLUS hardware registers. Some time ago, I had the opportunity to experiment with them and understand how they work (thanks to davee for pointing me to the base specs). Beyond the functions implemented by Sony, DMACPLUS allows transferring data in kernel mode to anywhere in the Media Engine's local EDRAM. The solution is fairly obvious from there: use DMACPLUS in word transfer mode to copy a single word, the new address of our custom function, directly into the Syscall table.

However, for the DMACPLUS to operate on the ME side, the corresponding bit in the bus clock enable register must first be set locally to enable clocking. To achieve this, I used a method I had in stock that restarts the ME while preserving the initial reset vector. This restart is necessary in order to pass the value to be written into the bus clock enable register via a shared register, a mechanism originally designed by Sony to update this register on the ME side by relaying information from the main CPU.

This method is a meaningful step forward. It opens the door to direct observation of the behavior of hardware components located on the Media Engine side.

VME and H.264 decoder, you are next!


## Final notes

The code library is regulary updated, please keep an eye on the repository!  
Thank you for reading.

*m-c/d*
