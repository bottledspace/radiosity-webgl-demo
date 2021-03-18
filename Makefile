.PHONY: html5 #win32

#INCLUDE_DIR = C:\src\vcpkg\installed\x86-windows\include
#LIB_DIR = C:\src\vcpkg\installed\x86-windows\lib
#LIBS = SDL2.lib GLEW32.lib OpenGL32.lib

#win32 : rad.cc
#	cl /std:c++latest /EHsc /I$(INCLUDE_DIR) rad.cc /link /LIBPATH:$(LIB_DIR) $(LIBS)
INCLUDE_DIR := /usr/local/Cellar/glm/0.9.9.8/include
LIB_DIR := 
html5 : rad.cc
	em++ -o rad.html --std=c++17 -s USE_SDL=2 -s USE_WEBGL2=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -I$(INCLUDE_DIR) rad.cc -L:$(LIB_DIR) --preload-file asset