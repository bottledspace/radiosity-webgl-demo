
<p align="center">
<img src="https://user-images.githubusercontent.com/46606990/122246624-b4478600-ce94-11eb-8698-ef5474804c61.png"></img>
</p>

# Radiosity WebGL Demonstration

Go to [the following link](https://astronotter.github.io/radiosity-webgl-demo/sol.html) to run the demo live in your browser. The demo requires WebGL2 to run which most browsers should support (Safari on ios is one notable exception).

This is a simulation of global illumination using the technique known as Radiosity. The light in this scene has been
calculated in real time almost entirely on the GPU through several shader passes, mostly
following the method presented in [Chapter 39](https://developer.nvidia.com/gpugems/gpugems2/part-v-image-oriented-computing/chapter-39-global-illumination-using-progressive)
of GPU Gems.
The demo is written in modern C++ using SDL and OpenGL, cross-compiled to WebASM using emscripten.
