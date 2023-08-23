
#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"

VM *initVM() {
  VM *vm = new VM();
  vm->stack = new Stack();
  vm->stack->init();
  return vm;
}

void freeVM() {}

static void runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);
  resetStack();
}

static uint8_t readByte(VM *vm) { return vm->instructions[vm->ip++]; }

static Value readConstant(VM *vm) { return vm->chunk->constants[readByte(vm)]; }

static Value peek(VM *vm, int distance) {
  return vm->stack->get(-1 - distance);
}

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
      if (!IS_NUMBER(peek(0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      vm->stack->push(NUMBER_VAL(-AS_NUMBER(*vm->stack->pop())));
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

InterpretResult interpret(VM *vm, std::string source) {
  Chunk *chunk = new Chunk();
  initChunk(chunk);
  if (!compile(source, chunk)) {
    freeChunk(chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm->chunk = chunk;
  vm->instructions = chunk->code;

  InterpretResult result = run(vm);

  freeChunk(chunk);
  return result;
}
