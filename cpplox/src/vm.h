#ifndef cpplox_vm_h
#define cpplox_vm_h

#include "chunk.h"
#include "object.h"
#include "stack.h"
#include <map>
#include <string>
#include <vector>

#define FRAMES_MAX 90

typedef struct {
  ObjFunction *function;
  std::vector<uint8_t> instructions;
  int ip;
  Stack *slots;
} CallFrame;

typedef struct {
  std::vector<CallFrame *> frames;
  int fp;
  std::map<std::string, Value> strings;
  std::map<std::string, Value> globals;
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
