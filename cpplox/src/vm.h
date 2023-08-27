#ifndef cpplox_vm_h
#define cpplox_vm_h

#include "chunk.h"
#include "stack.h"
#include <map>
#include <string>

typedef struct {
  Chunk *chunk;
  std::vector<uint8_t> instructions;
  std::map<std::string, Value> strings;
  std::map<std::string, Value> globals;
  int ip;
  Stack *stack;
} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

VM *initVM();
void freeVM();
InterpretResult interpret(VM *vm, std::string source);

#endif
