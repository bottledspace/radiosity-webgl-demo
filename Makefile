

INCLUDE_DIRS := \
	-I"C:\src\vcpkg\installed\x86-windows\include"
LIB_DIRS := 
sol.html : *.cc *.hpp asset/*
	em++ -o sol.html -O3 --std=c++17 -s USE_SDL=2 -s USE_WEBGL2=1 $(INCLUDE_DIRS) \
		--shell-file shell.html sol.cc $(LIB_DIRS) --embed-file asset
