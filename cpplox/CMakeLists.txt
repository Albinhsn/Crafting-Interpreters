cmake_minimum_required(VERSION 3.14)
set(This cpplox)
project(${This})

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(googletest)

set(Sources
  # ./src/scanner.cpp
  # ./src/scanner.h
  ./src/debug.cpp
  ./src/debug.h
  ./src/chunk.cpp
  ./src/chunk.h
  ./src/memory.cpp
  ./src/memory.h
  ./src/value.cpp
  ./src/value.h
  ./src/vm.cpp
  ./src/stack.h
  ./src/vm.h
  # ./src/compiler.h
  # ./src/compiler.cpp
  ./src/main.cpp
  ./src/common.h
  )
add_executable(
  main ${Sources}
  )

add_library(${This} STATIC ${Sources})
