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
  int sp; // Stack pointer
} CallFrame;

typedef struct {
  std::vector<CallFrame *> frames;
  std::map<std::string, Value> strings;
  std::map<std::string, Value> globals;
  std::vector<Obj *> objects;
  Stack *stack;
} VM;

extern VM *vm;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(std::string source);

#endif
