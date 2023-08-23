#ifndef cpplox_vm_h
#define cpplox_vm_h

#include "chunk.h"
#include "stack.h"

typedef struct {
  Chunk *chunk;
  std::vector<uint8_t> instructions;
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
InterpretResult interpret(Chunk *chunk, VM *vm);

#endif
