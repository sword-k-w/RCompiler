build:
	cmake cmake-build-debug
	cmake --build cmake-build-debug

run: build
	./cmake-build-debug/code