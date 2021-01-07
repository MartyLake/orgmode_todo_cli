test: test.cpp build
	cmake --build build --target main_test && ./build/main_test
run: main
	./main
build:
	(mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug)
main: main.cpp build
	(cd build && cmake --build . --target main && cp main .. && cd .. && ctags main.cpp orgparse.cpp)
clean:
	rm -rf build
	rm main
