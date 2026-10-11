## Me MIST - VME MIST

This is a first PoC with custom code processing data over the VME. It demonstrates the VME flow and how to perform an operation on a set of 32 values through a 3-stage fixed-point Q.23 pipeline that multiplies each value of a buffer by a variable factor.

It uses the MIST method to execute code on the Media Engine. You will need to install the `me-mist` library and the `me-custom-core` library to compile this project. Both libraries are still under development, to integrate VME-related tools on the `me-custom-core` side and as a new library for `me-mist`.

Compatibility is currently limited to Slim+ devices, since core mapping still needs to be updated for the older devices.
 
## Disclamer
This project and code are provided as-is without warranty. Users assume full responsibility for any implementation or consequences. Use at your own discretion and risk


*m-c/d*
