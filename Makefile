INCLUDE_DIRS := \
	-I/usr/local/Cellar/glm/0.9.9.8/include \
	-I/usr/local/Cellar/emscripten/2.0.14/libexec/system 
LIB_DIRS := 
sol.html : shell.html *.cc *.hpp asset/*
	em++ -o sol.html -O3 --std=c++17 -s USE_SDL=2 -s USE_WEBGL2=1 $(INCLUDE_DIRS) \
		sol.cc $(LIB_DIRS) --shell-file shell.html --preload-file asset
