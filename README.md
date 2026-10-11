## PSP Media Engine MIST
A library to safely execute custom tasks on the Media Engine using the MIST method (Media Engine Injected Syscall using Transfer), without disrupting syscalls dispatched to it by the SC.  

- Tasks are executed in parallel without blocking the SC
- The task is invoked through a custom address injected directly into the Syscall table via DMACPLUS, without patching the core
- No manual soft reset is required, the method is self-sustained
- Device sleep works properly as the base ME wrapper is not altered
- Compatible with XMB, allowing custom tasks to be triggered over it

Note: MIST tested on Slim and Phat.

### Advantages

- `sleep` and `awake` are functional
- Syscalls from SC to the ME are preserved
- Tasks can be triggered through the SC threads
- A wait function is available to ensure a previously triggered custom task has completed, other SC processes can be placed between the trigger and the task's end within the same thread
- The `me-core` mapped functions are available and can be invoked, as well as `me-core-lib` functions

It remains possible to call the ME as a single parallel task, but you must ensure the loop is broken before any `sleep`/`awake` event or before making another syscall to the ME.

## Requirement
- Clone and install psp-media-engine-custom-core library, see below.
- Clone this repository, then build and install the library via cmake.

## build
using prx
```bash
cmake -DPRX_FREE=0 ..
make clean; make install;
```

using kubridge (no prx)
```bash
cmake -DPRX_FREE=1 ..
make clean; make install;
```

## Usage
See samples directory.

## MIST
see [here](MIST.md).

*Note: all `sceUtilityLoadAvModule` calls should be done after injections*

## Code samples

Code samples are available in the `samples` folder: a PoC for the MIST method, a PoC for the Easy MIST helpers, and the first VME custom process (`poc-mist-vme`) processing data through the PSP DSP. The VME sample also needs `vme-lib` from the `me-core-mapper` package, which is a separate library.

## Technical Note

This project uses custom code, original work that was not derived from reverse engineering. Please keep in mind that these implementations are based on the author's understanding of the hardware at the time the code was written.

## Contribution Guidelines

### AI-assisted development

AI tools may be used as development aids. However, the following rules apply strictly:

* All commits must be authored by a human contributor (pseudonyms are perfectly acceptable).
* The commit history must not contain any AI attribution as author or co-author.
* Contributors must fully review, understand, and validate all submitted code before opening a pull request.
* Contributors are expected to be able to explain and justify their changes during code review.
* The contributor is responsible for ensuring their code does not break existing functionality, including dependencies and the overall library behavior.

*In short: AI can assist, but humans must retain full ownership of the work.*

Pull requests that include AI attribution in commits, or that are not clearly understood and validated by the contributor, will be rejected.

### License compatibility

All code submitted to this repository must be compatible with the MIT License. Dependencies or code snippets under more restrictive licenses (e.g. GPL, LGPL, proprietary) are not accepted. Contributors are responsible for verifying that any third-party code they include is under a permissive license granting at least the same level of freedom as MIT.

## License
This project is released under the MIT License, see [LICENSE.md](LICENSE.md).

## Disclamer
This project and code are provided as-is without warranty. Users assume full responsibility for any implementation or consequences. Use at your own discretion and risk

## Related work
[PSP Media Engine Reload](https://github.com/mcidclan/psp-media-engine-reload)  
[PSP Media Engine Custom Core](https://github.com/mcidclan/psp-media-engine-custom-core)  
[PSP Media Engine Cracking the unknown](https://github.com/mcidclan/psp-media-engine-cracking-the-unknown)  

*m-c/d*
