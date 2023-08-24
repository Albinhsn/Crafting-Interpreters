
#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include <cstdarg>

VM *initVM() {
  VM *vm = new VM();
  vm->stack = new Stack();
  vm->stack->init();
  return vm;
}

void freeVM() {}

static void resetStack(VM *vm) {
  delete (vm->stack);
  vm->stack = new Stack();
}

static void runtimeError(VM *vm, std::string format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format.c_str(), args);
  va_end(args);
  fputs("\n", stderr);

  // size_t instruction = vm->ip - vm->chunk->code - 1;
  int line = vm->chunk->lines[vm->ip];
  fprintf(stderr, "[line %d] in script\n", line);
  resetStack(vm);
}

static uint8_t readByte(VM *vm) { return vm->instructions[vm->ip++]; }

static Value readConstant(VM *vm) { return vm->chunk->constants[readByte(vm)]; }

static Value peek(VM *vm, int distance) { return vm->stack->get(distance); }

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

InterpretResult run(VM *vm) {
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {                  \
      runtimeError(vm, "Operands must be numbers.");                           \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    std::cout << "GOT\n";                                                      \
    double b = AS_NUMBER(*vm->stack->pop());                                   \
    double a = AS_NUMBER(*vm->stack->pop());                                   \
    vm->stack->push(valueType(a op b));                                        \
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
    case OP_NIL: {
      vm->stack->push(NIL_VAL);
      break;
    }
    case OP_TRUE: {
      std::cout << "Pushing true\n";
      vm->stack->push(BOOL_VAL(true));
      break;
    }
    case OP_FALSE: {
      vm->stack->push(BOOL_VAL(false));
      break;
    }
    case OP_EQUAL: {
      Value b = *vm->stack->pop();
      Value a = *vm->stack->pop();
      vm->stack->push(BOOL_VAL(valuesEqual(a, b)));
      break;
    }
    case OP_GREATER: {
      BINARY_OP(BOOL_VAL, >);
      break;
    }
    case OP_LESS: {
      BINARY_OP(BOOL_VAL, <);
      break;
    }
    case OP_ADD: {
      BINARY_OP(NUMBER_VAL, +);
      break;
    }
    case OP_SUBTRACT: {
      BINARY_OP(NUMBER_VAL, -);
      break;
    }
    case OP_MULTIPLY: {
      BINARY_OP(NUMBER_VAL, *);
      break;
    }
    case OP_DIVIDE: {
      BINARY_OP(NUMBER_VAL, /);
      break;
    }
    case OP_NOT: {
      vm->stack->push(BOOL_VAL(isFalsey(*vm->stack->pop())));
      break;
    }
    case OP_NEGATE: {
      if (!IS_NUMBER(peek(vm, 0))) {
        runtimeError(vm, "Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      vm->stack->push(NUMBER_VAL(-AS_NUMBER(*vm->stack->pop())));
      break;
    }
    case OP_RETURN: {
      printValue(*vm->stack->pop());
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
