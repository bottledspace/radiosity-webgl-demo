
<p align="center">
<img src="https://user-images.githubusercontent.com/46606990/122246624-b4478600-ce94-11eb-8698-ef5474804c61.png"></img>
</p>

# Radiosity WebGL Demonstration

A simulation of global illumination using Radiosity. The light in this scene has been
calculated in real time almost entirely on the GPU through several shader passes, mostly
following the method presented in [Chapter 39](https://developer.nvidia.com/gpugems/gpugems2/part-v-image-oriented-computing/chapter-39-global-illumination-using-progressive)
of GPU Gems.

The demo is written in modern C++ using SDL and OpenGL and can be compiled as a desktop application (Windows, OSX, Linux, etc)
or cross-compiled to WebGL2, using emscripten. Please note that not all browsers currently support WebGL2 (looking at you Apple!)
