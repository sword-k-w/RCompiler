build:
	mkdir build
	cd build && cmake .. && cmake --build .

run: build
	./build/code