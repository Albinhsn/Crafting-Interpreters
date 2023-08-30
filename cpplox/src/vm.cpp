
#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "value.h"
#include <cstdarg>
#include <cstring>
#include <map>
#include <time.h>

static void defineNative(VM *vm, std::string name, NativeFn function);

static Value clockNative(int argCount, Value args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

VM *initVM() {
  VM *vm = new VM();
  vm->stack = new Stack();
  vm->stack->init();
  vm->strings = std::map<std::string, Value>();
  vm->globals = std::map<std::string, Value>();

  defineNative(vm, "clock", clockNative);

  return vm;
}
static CallFrame *currentFrame(VM *vm) {
  return vm->frames[vm->frames.size() - 1];
}

void freeVM() {}

static void resetStack(VM *vm) {
  delete (vm->stack);
  vm->stack = new Stack();
  vm->frames = std::vector<CallFrame *>();
}

static void runtimeError(VM *vm, std::string format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format.c_str(), args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = vm->frames.size() - 1; i >= 0; i--) {
    CallFrame *frame = vm->frames[i];
    ObjFunction *function = frame->function;
    size_t instruction = frame->instructions[frame->ip];
    fprintf(stderr, "[line %d] in ", function->chunk->lines[instruction]);

    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars.c_str());
    }
  }

  resetStack(vm);
}

static void defineNative(VM *vm, std::string name, NativeFn function) {
  vm->stack->push(OBJ_VAL(copyString(name)));
  vm->stack->push(OBJ_VAL(newNative(function)));

  vm->globals[AS_STRING(vm->stack->get(1))->chars] = vm->stack->get(0);

  vm->stack->pop();
  vm->stack->pop();
}

static uint8_t readByte(VM *vm) {
  CallFrame *frame = currentFrame(vm);
  return frame->instructions[frame->ip++];
}

static Value readConstant(VM *vm) {
  return currentFrame(vm)->function->chunk->constants[readByte(vm)];
}

static Value peek(VM *vm, int distance) { return vm->stack->get(distance); }

static bool call(VM *vm, ObjFunction *function, int argCount) {
  if (argCount != function->arity) {
    runtimeError(vm, "Expected %d arguments but got %d", function->arity,
                 argCount);
    return false;
  }

  if (vm->frames.size() == FRAMES_MAX) {
    runtimeError(vm, "Stack overflow.");
    return false;
  }
  CallFrame *frame = new CallFrame();
  frame->function = function;
  frame->instructions = function->chunk->code;
  frame->ip = 0;
  frame->sp = vm->stack->length - argCount - 1;
  vm->frames.push_back(frame);
  return true;
}

static bool callValue(VM *vm, Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
    case OBJ_NATIVE: {
      NativeFn native = AS_NATIVE(callee);
      Value result =
          native(argCount, vm->stack->get(vm->stack->length - argCount));
      vm->stack->length -= argCount + 1;
      vm->stack->push(result);
      return true;
    }
    case OBJ_FUNCTION: {
      return call(vm, AS_FUNCTION(callee), argCount);
    }
    default:
      break;
    }
  }
  if (OBJ_TYPE(callee) == OBJ_STRING) {
    std::cout << "Trying to call string " << AS_STRING(callee) << "\n";
  }
  runtimeError(vm, "Can only call functions and classes.");
  return false;
}

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate(VM *vm) {
  std::string a = AS_STRING(vm->stack->pop())->chars;
  std::string b = AS_STRING(vm->stack->pop())->chars;
  std::string c = b.append(a);
  Value value = OBJ_VAL(copyString(c));
  vm->stack->push(value);
}

InterpretResult run(VM *vm) {
  CallFrame *frame = currentFrame(vm);
#define READ_SHORT()                                                           \
  (frame->ip += 2, (uint16_t)((frame->instructions[frame->ip - 2] << 8) |      \
                              frame->instructions[frame->ip - 1]))
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
    for (int slot = vm->stack->length - 1; slot >= 0; slot--) {
      std::cout << "[ ";
      printValue(vm->stack->get(slot));
      std::cout << " ]";
    }
    std::cout << "\n";
    disassembleInstruction(frame->function->chunk, (int)frame->ip);
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
    case OP_GET_LOCAL: {
      uint8_t slot = readByte(vm);
      vm->stack->push(vm->stack->get((vm->stack->length  - 2 - frame->sp) - slot ));
      break;
    }
    case OP_SET_LOCAL: {
      uint8_t slot = readByte(vm);
      vm->stack->update(frame->sp + slot, peek(vm, 0));
      break;
    }
    case OP_GET_GLOBAL: {
      std::string name = AS_STRING(readConstant(vm))->chars;
      if (!vm->globals.count(name)) {
        runtimeError(vm, "Undefined variable '" + name + "'.");
        return INTERPRET_RUNTIME_ERROR;
      }
      Value value = vm->globals[name];
      vm->stack->push(value);
      break;
    }
    case OP_DEFINE_GLOBAL: {
      std::string s = AS_STRING(readConstant(vm))->chars;
      vm->globals[s] = peek(vm, 0);
      vm->stack->pop();
      break;
    }
    case OP_SET_GLOBAL: {
      std::string name = AS_STRING(readConstant(vm))->chars;
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
    case OP_JUMP: {
      uint16_t offset = READ_SHORT();
      frame->ip += offset;
      break;
    }
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = READ_SHORT();
      if (isFalsey(peek(vm, 0))) {
        frame->ip += offset;
      }
      break;
    }
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      frame->ip -= offset;
      break;
    }
    case OP_CALL: {
      int argCount = readByte(vm);
      if (!callValue(vm, peek(vm, argCount), argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = vm->frames[vm->frames.size() - 1];
      break;
    }
    case OP_RETURN: {
      Value result = vm->stack->pop();
      vm->frames.pop_back();
      if (vm->frames.size() == 0) {
        vm->stack->pop();
        return INTERPRET_OK;
      }

      vm->stack->remove(frame->sp);
      vm->stack->push(result);
      frame = vm->frames[vm->frames.size() - 1];
      break;
    }
    }
  }
#undef READ_SHORT
#undef BINARY_OP
}

InterpretResult interpret(VM *vm, std::string source) {
  Chunk *chunk = new Chunk();
  initChunk(chunk);
  ObjFunction *function = compile(source, chunk);
  if (function == NULL) {
    return INTERPRET_COMPILE_ERROR;
  }

  vm->stack->push(OBJ_VAL(function));
  call(vm, function, 0);
  std::cout << "\n== Running in vm == \n";
  return run(vm);
}
