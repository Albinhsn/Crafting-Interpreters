i:
	echo "Idk not fixed yet"

cd:
	cd src/ && g++ -g -fsanitize=address,undefined -o main main.cpp chunk.cpp memory.cpp debug.cpp value.cpp vm.cpp compiler.cpp scanner.cpp object.cpp
crd:
	cd src/ && g++  -g -fsanitize=address,undefined -o main main.cpp chunk.cpp memory.cpp debug.cpp value.cpp vm.cpp compiler.cpp scanner.cpp object.cpp && ./main

cfd: 
	cd src/ && g++ -g -fsanitize=address,undefined -o main main.cpp chunk.cpp memory.cpp debug.cpp value.cpp vm.cpp compiler.cpp scanner.cpp object.cpp && ./main ../input

c:
	cd src/ && g++ -g -o main main.cpp chunk.cpp memory.cpp debug.cpp value.cpp vm.cpp compiler.cpp scanner.cpp object.cpp
cr:
	cd src/ && g++  -g -o main main.cpp chunk.cpp memory.cpp debug.cpp value.cpp vm.cpp compiler.cpp scanner.cpp object.cpp && ./main

cf: 
	cd src/ && g++ -g -o main main.cpp chunk.cpp memory.cpp debug.cpp value.cpp vm.cpp compiler.cpp scanner.cpp object.cpp && ./main ../input

bt:
	cmake -S . -B build && cmake --build build
	cd build/ && ctest --output-on-failure -V
t:
	cd build/ && ctest  --output-on-failure -V 

br:
	cmake -S . -B build && cmake --build build && cd build/ && ./main $(file)

r:
	cmake --build build && cd build/ && ./main $(file)
