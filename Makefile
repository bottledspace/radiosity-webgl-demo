.PHONY: win32
INCLUDE_DIRS := \
	/I "C:\src\vcpkg\installed\x86-windows\include" \
	/I "C:\Program Files\RenderDoc"
LIB_DIR := C:\src\vcpkg\installed\x86-windows\lib
LIBS := SDL2.lib GLEW32.lib OpenGL32.lib

win32 : rad.cc
	cl /std:c++latest /EHsc $(INCLUDE_DIRS) rad.cc /link /LIBPATH:$(LIB_DIR) $(LIBS)