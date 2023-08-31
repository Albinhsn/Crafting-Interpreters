#include "chunk.h"
#include "common.h"
#include "coz.h"
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

static void runFile(std::string path) {
  std::string source = readFile(path);
  InterpretResult result = interpret(source);

  freeVM();
  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

static void repl() {
  std::string input;
  for (;;) {
    std::cout << "> ";
    std::getline(std::cin, input);
    if (input == "q") {
      printf("\n");
      break;
    }

    interpret(input);
  }
  freeVM();
}

int main(int argc, const char *argv[]) {
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    std::fprintf(stderr, "Usage: clox [path]\n");
    exit(64);
  }
  return 0;
}
