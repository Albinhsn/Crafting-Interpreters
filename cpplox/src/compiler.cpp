

#include "compiler.h"
#include "common.h"
#include "scanner.h"

static void advance(){
}

bool compile(std::string source, Chunk * chunk) {
  initScanner(source);
  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");
}
