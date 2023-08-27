i:
	echo "Idk not fixed yet"

cr:
	cd src/ && g++ -o main main.cpp chunk.cpp memory.cpp debug.cpp value.cpp vm.cpp compiler.cpp scanner.cpp object.cpp && ./main

cf: 
	cd src/ && g++ -o main main.cpp chunk.cpp memory.cpp debug.cpp value.cpp vm.cpp compiler.cpp scanner.cpp object.cpp && ./main ../$(file)

bt:
	cmake -S . -B build && cmake --build build
	cd build/ && ctest --output-on-failure -V
t:
	cd build/ && ctest  --output-on-failure -V 

br:
	cmake -S . -B build && cmake --build build && cd build/ && ./main $(file)

r:
	cmake --build build && cd build/ && ./main $(file)
