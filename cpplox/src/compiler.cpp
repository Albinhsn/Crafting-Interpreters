

#include "compiler.h"
#include "common.h"
#include "scanner.h"

void compile(std::string source) {
  initScanner(source);
  int line = -1;
  for (;;) {
    Token token = scanToken();
    if (token.line != line) {
      std::cout << token.line << " ";
      line = token.line;
    } else {
      std::cout << "    | ";
    }
    std::cout << token.type << " '" << token.literal << "'\n";

    if (token.type == TOKEN_EOF)
      break;
  }
}
