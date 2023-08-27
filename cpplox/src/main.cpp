#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"
#include <cstdio>
#include <fstream>
#include <sstream>

static std::string readFile(std::string path) {
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  t.close();
  return buffer.str();
}

static void runFile(VM *vm, std::string path) {
  std::string source = readFile(path);
  InterpretResult result = interpret(vm, source);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

static void repl(VM *vm) {
  std::string input;
  for (;;) {
    std::cout << "> ";
    std::getline(std::cin, input);
    if (input == "q") {
      printf("\n");
      break;
    }

    interpret(vm, input);
  }
}

int main(int argc, const char *argv[]) {
  VM *vm = initVM();
  if (argc == 1) {
    repl(vm);
  } else if (argc == 2) {
    runFile(vm, argv[1]);
  } else {
    std::fprintf(stderr, "Usage: clox [path]\n");
    exit(64);
  }
  freeVM();
  return 0;
}
