build:
	cmake build
	cmake --build build

run: build
	./build/code