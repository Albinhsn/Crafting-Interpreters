
#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "value.h"
#include <cstdarg>
#include <cstring>
#include <map>

VM *initVM() {
  VM *vm = new VM();
  vm->stack = new Stack();
  vm->stack->init();
  vm->strings = std::map<std::string, Value>();
  vm->globals = std::map<std::string, Value>();

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

static void concatenate(VM *vm) {
  std::string a = AS_STRING(vm->stack->pop());
  std::string b = AS_STRING(vm->stack->pop());
  std::string c = b.append(a);
  Value value = STRING_VAL(c.c_str());
  vm->stack->push(value);
}

InterpretResult run(VM *vm) {
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {                  \
      runtimeError(vm, "Operands must be numbers.");                           \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    double b = AS_NUMBER(vm->stack->pop());                                    \
    double a = AS_NUMBER(vm->stack->pop());                                    \
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
    case OP_POP: {
      vm->stack->pop();
      break;
    }
    case OP_GET_GLOBAL: {
      const char *name = AS_STRING(readConstant(vm));
      if (!vm->globals.count(name)) {
        std::string stdname = name;
        runtimeError(vm, "Undefined variable '" + stdname + "'.");
        return INTERPRET_RUNTIME_ERROR;
      }
      Value value = vm->globals[name];
      vm->stack->push(value);
      break;
    }
    case OP_DEFINE_GLOBAL: {
      vm->globals[AS_STRING(readConstant(vm))] = peek(vm, 0);
      vm->stack->pop();
      break;
    }
    case OP_SET_GLOBAL: {
      const char *name = AS_STRING(readConstant(vm));
      if (!vm->globals.count(name)) {
        std::string msg = name;
        runtimeError(vm, "Undefined variable '" + msg + "'.");
        return INTERPRET_RUNTIME_ERROR;
      }
      vm->globals[name] = peek(vm, 0);
      break;
    }
    case OP_EQUAL: {
      Value b = vm->stack->pop();
      Value a = vm->stack->pop();
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
      if (IS_STRING(peek(vm, 0)) && IS_STRING(peek(vm, 1))) {
        concatenate(vm);
      } else if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
        double b = AS_NUMBER(vm->stack->pop());
        double a = AS_NUMBER(vm->stack->pop());
        vm->stack->push(NUMBER_VAL(a + b));
      } else {
        runtimeError(vm, "Operands must be two number or two strings");
        return INTERPRET_RUNTIME_ERROR;
      }
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
      vm->stack->push(BOOL_VAL(isFalsey(vm->stack->pop())));
      break;
    }
    case OP_NEGATE: {
      if (!IS_NUMBER(peek(vm, 0))) {
        runtimeError(vm, "Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      vm->stack->push(NUMBER_VAL(-AS_NUMBER(vm->stack->pop())));
      break;
    }
    case OP_PRINT: {
      printValue(vm->stack->pop());
      std::cout << "\n";
      break;
    }
    case OP_RETURN: {
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

  std::cout << "\n== Running in vm == ";
  InterpretResult result = run(vm);

  freeChunk(chunk);
  vm->instructions = std::vector<uint8_t>();
  return result;
}
