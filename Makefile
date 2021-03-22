.PHONY: html5 #win32

#INCLUDE_DIR = C:\src\vcpkg\installed\x86-windows\include
#LIB_DIR = C:\src\vcpkg\installed\x86-windows\lib
#LIBS = SDL2.lib GLEW32.lib OpenGL32.lib

#win32 : rad.cc
#	cl /std:c++latest /EHsc /I$(INCLUDE_DIR) rad.cc /link /LIBPATH:$(LIB_DIR) $(LIBS)
INCLUDE_DIRS := \
	-I/usr/local/Cellar/glm/0.9.9.8/include \
	-I/usr/local/Cellar/emscripten/2.0.14/libexec/system 
LIB_DIRS := 
html5 : rad.cc shell.html
	em++ -o rad.html -g4 --source-map-base --std=c++17 -s USE_SDL=2 -s USE_WEBGL2=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 $(INCLUDE_DIRS) rad.cc $(LIB_DIRS) --shell-file shell.html --preload-file asset