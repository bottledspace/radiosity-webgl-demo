# sol

A simulation of global illumination using Radiosity. The light in this scene has been
calculated in real time almost entirely on the GPU through several shader passes, mostly
following the method presented in [Chapter 39](https://developer.nvidia.com/gpugems/gpugems2/part-v-image-oriented-computing/chapter-39-global-illumination-using-progressive)
of GPU Gems.

<img src="https://i.imgur.com/DTs8vpW.mp4"></img>

The demo is written in modern C++ using SDL and OpenGL and can be compiled as a desktop application (Windows, OSX, Linux, etc)
or cross-compiled to WebGL2, using emscripten. Please note that not all browsers currently support WebGL2 (looking at you Apple!)
