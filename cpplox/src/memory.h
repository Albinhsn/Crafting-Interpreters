#ifndef cpplox_memory_h
#define cpplox_memory_h

#include <cstddef>
#include "object.h"
#include "scanner.h"
#include "compiler.h"
#include "vm.h"

void freeParser(Parser * parser);
void freeScanner(Scanner * scanner);
void freeCompiler(Compiler * compiler);
void freeObjects();
int freeFrame(VM * vm);

#endif
