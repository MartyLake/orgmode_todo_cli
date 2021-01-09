test: build/main_test
	./build/main_test
build/main_test: *.cpp *.hpp
	cmake --build build --target main_test
run: main
	./main
build:
	(mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug)
main: main.cpp build
	(cd build && cmake --build . --target main && cp main .. && cp -f main $$HOME/bin/orgparse_todo_cli && cd .. && ctags main.cpp orgparse.cpp)
clean:
	rm -rf build
	rm -f main
