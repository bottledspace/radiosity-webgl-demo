.PHONY: html5 win32
INCLUDE_DIR = C:\src\vcpkg\installed\x86-windows\include
LIB_DIR = C:\src\vcpkg\installed\x86-windows\lib
LIBS = SDL2.lib GLEW32.lib OpenGL32.lib

win32 : rad.cc
	cl /std:c++latest /EHsc /I$(INCLUDE_DIR) rad.cc /link /LIBPATH:$(LIB_DIR) $(LIBS)
html5 : rad.cc
	em++ -I$(INCLUDE_DIR) rad.cc -L:$(LIB_DIR)