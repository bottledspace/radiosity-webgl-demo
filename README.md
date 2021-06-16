# sol

A simulation of global illumination using Radiosity. The light in this scene has been
calculated in real time almost entirely on the GPU through several shader passes, mostly
following the method presented in [Chapter 39](https://developer.nvidia.com/gpugems/gpugems2/part-v-image-oriented-computing/chapter-39-global-illumination-using-progressive)
of GPU Gems.

The demo can be compiled as a desktop application (Windows, OSX, Linux, etc) or cross
compiled to WebGL2, using emscripten.
