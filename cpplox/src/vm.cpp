
#include "vm.h"
#include "common.h"
#include "debug.h"

VM *initVM() {
  VM *vm = new VM();
  vm->stack = new Stack();
  vm->stack->init();
  return vm;
}

void freeVM() {}

static uint8_t readByte(VM *vm) { return vm->instructions[vm->ip++]; }

static Value readConstant(VM *vm) { return vm->chunk->constants[readByte(vm)]; }

InterpretResult run(VM *vm) {
#define BINARY_OP(op)                                                          \
  do {                                                                         \
    double b = vm->stack->pop();                                               \
    double a = vm->stack->pop();                                               \
    vm->stack->push(a op b);                                                   \
  } while (false)

  for (;;) {

#ifdef DEBUG_TRACE_EXECUTION
    std::cout << "        ";
    for (int slot = 0; slot < vm->stack->length; slot++) {
      std::cout << "[ ";
      printValue(vm->stack->get(slot));
      std::cout << " ]";
    }
    std::cout << "\n";
    disassembleInstruction(vm->chunk, vm->ip);
#endif
    switch (readByte(vm)) {
    case OP_CONSTANT: {
      Value constant = readConstant(vm);
      vm->stack->push(constant);
      break;
    }
    case OP_ADD: {
      BINARY_OP(+);
      break;
    }
    case OP_SUBTRACT: {
      BINARY_OP(-);
      break;
    }
    case OP_MULTIPLY: {
      BINARY_OP(*);
      break;
    }
    case OP_DIVIDE: {
      BINARY_OP(/);
      break;
    }
    case OP_NEGATE: {
      vm->stack->push(-vm->stack->pop());
      break;
    }
    case OP_RETURN: {
      printValue(vm->stack->pop());
      std::cout << "\n";
      return INTERPRET_OK;
    }
    }
  }
}

InterpretResult interpret(Chunk *chunk, VM *vm) {
  vm->chunk = chunk;
  vm->instructions = chunk->code;
  vm->ip = 0;

  return run(vm);
}
