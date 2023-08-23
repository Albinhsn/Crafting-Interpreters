i:
	echo "Idk not fixed yet"

c:
	cd src/ && g++ -o main main.cpp chunk.cpp memory.cpp debug.cpp value.cpp vm.cpp compiler.cpp scanner.cpp && ./main

bt:
	cmake -S . -B build && cmake --build build
	cd build/ && ctest --output-on-failure -V
t:
	cd build/ && ctest  --output-on-failure -V 

br:
	cmake -S . -B build && cmake --build build && cd build/ && ./main $(file)

r:
	cmake --build build && cd build/ && ./main $(file)
