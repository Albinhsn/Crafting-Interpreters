
#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "stack.h"
#include "value.h"
#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <map>
#include <time.h>

VM *vm;

static void defineNative(std::string name, NativeFn function);

static Value clockNative(int argCount, Value args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

void initVM() {
  vm = new VM();
  vm->stack = new Stack();
  vm->stack->init();
  vm->strings = std::map<std::string, Value>();
  vm->globals = std::map<std::string, Value>();
  vm->objects = std::vector<Obj *>();

  defineNative("clock", clockNative);
}
static CallFrame *currentFrame() { return vm->frames[vm->frames.size() - 1]; }

void freeVM() { vm->frames.clear(); }

static void resetStack(VM *vm) {
  vm->stack = new Stack();
  vm->stack->init();
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

static void defineNative(std::string name, NativeFn function) {
  ObjString *string = copyString(name);
  vm->objects.push_back((Obj *)string);
  vm->stack->push(OBJ_VAL(string));

  ObjNative *native = newNative(function);
  vm->objects.push_back((Obj *)native);
  vm->stack->push(OBJ_VAL(native));

  vm->globals[AS_STRING(vm->stack->get(1))->chars] = vm->stack->get(0);

  vm->stack->pop();
  vm->stack->pop();
}

static uint8_t readByte() {
  CallFrame *frame = currentFrame();
  return frame->instructions[frame->ip++];
}

static bool matchByte(OpCode code) {
  CallFrame *frame = currentFrame();
  if (frame->instructions[frame->ip] == code) {
    frame->ip++;
    return true;
  }
  return false;
}

static Value readConstant() {
  return currentFrame()->function->chunk->constants[readByte()];
}

static Value peek(int distance) { return vm->stack->get(distance); }

static bool call(ObjFunction *function, int argCount) {
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

static bool callValue(Value callee, int argCount) {
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
      return call(AS_FUNCTION(callee), argCount);
    }
    case OBJ_STRUCT: {
      ObjStruct *strukt = AS_STRUCT(callee);
      if (strukt->fields.size() != argCount) {
        runtimeError(vm, "Expected %d argument for struct but got %d",
                     strukt->fields.size(), argCount);
        return false;
      }
      std::vector<Value> fields;
      for (int i = 0; i < argCount; ++i) {
        fields.push_back(vm->stack->pop());
      }
      vm->stack->update(0, OBJ_VAL(newInstance(strukt, fields)));
      return true;
    }
    default:
      break;
    }
  }
  runtimeError(vm, "Can only call functions and classes.");
  return false;
}

static bool index() {
  Value key = vm->stack->pop();
  Value item = vm->stack->pop();
  if (item.type != VAL_OBJ) {
    runtimeError(vm, "Can't only index array, map and string");
    return false;
  }
  switch (OBJ_TYPE(item)) {
  case OBJ_MAP: {
    ObjMap *mp = AS_MAP(item);
    ObjString *string = AS_STRING(key);

    if (mp->m.count(string->chars)) {
      vm->stack->push(mp->m[string->chars]);
      return true;
    }
    runtimeError(vm, "Trying to access map with unknown key %s",
                 string->chars.c_str());
    return false;
  }
  case OBJ_STRING: {
    if (key.type != VAL_NUMBER) {
      runtimeError(vm, "Can only index string with number");
      return false;
    }
    ObjString *string = AS_STRING(item);
    int k = (int)key.as.number;
    if (string->chars.size() <= k || k < 0) {
      runtimeError(vm, "Trying to access outside of array %d", k);
      return false;
    }

    Value value;
    value.type = VAL_OBJ;
    value.as.obj = (Obj *)copyString(string->chars.substr(k, 1));
    vm->stack->push(value);
    return true;
  }
  case OBJ_ARRAY: {
    if (key.type != VAL_NUMBER) {
      runtimeError(vm, "Can only index array with number");
      return false;
    }
    int k = (int)key.as.number;
    ObjArray *array = AS_ARRAY(item);
    if (array->values.size() <= k || k < 0) {
      runtimeError(vm, "Trying to access outside of array %d", k);
      return false;
    }
    vm->stack->push(array->values[k]);
    return true;
  }
  default: {
    runtimeError(vm, "Can't only index array, map and string");
    return false;
  }
  }
}

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
  std::string a = AS_STRING(vm->stack->pop())->chars;
  std::string b = AS_STRING(vm->stack->pop())->chars;
  std::string c = b.append(a);
  Value value = OBJ_VAL(copyString(c));
  vm->stack->push(value);
}

InterpretResult run() {
  CallFrame *frame = currentFrame();
#define READ_SHORT()                                                           \
  (frame->ip += 2, (uint16_t)((frame->instructions[frame->ip - 2] << 8) |      \
                              frame->instructions[frame->ip - 1]))
#define READ_STRING() AS_STRING(readConstant())
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
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
    switch (readByte()) {
    case OP_CONSTANT: {
      Value constant = readConstant();
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
      uint8_t slot = readByte();
      vm->stack->push(
          vm->stack->get((vm->stack->length - 2 - frame->sp) - slot));
      break;
    }
    case OP_SET_LOCAL: {
      uint8_t slot = readByte();
      vm->stack->update(frame->sp + slot, peek(0));
      break;
    }
    case OP_GET_GLOBAL: {
      std::string name = READ_STRING()->chars;
      if (!vm->globals.count(name)) {
        runtimeError(vm, "Undefined variable '" + name + "'.");
        return INTERPRET_RUNTIME_ERROR;
      }
      Value value = vm->globals[name];
      vm->stack->push(value);
      break;
    }
    case OP_DEFINE_GLOBAL: {
      std::string s = READ_STRING()->chars;
      vm->globals[s] = peek(0);
      vm->stack->pop();
      break;
    }
    case OP_SET_GLOBAL: {
      std::string name = READ_STRING()->chars;
      if (!vm->globals.count(name)) {
        std::string msg = name;
        runtimeError(vm, "Undefined variable '" + msg + "'.");
        return INTERPRET_RUNTIME_ERROR;
      }
      vm->globals[name] = peek(0);
      break;
    }
    case OP_GET_PROPERTY: {
      if (!IS_INSTANCE(peek(0))) {
        runtimeError(vm, "Only instances have properties.");
        return INTERPRET_RUNTIME_ERROR;
      }
      // Get the instance from the top
      ObjInstance *instance = AS_INSTANCE(peek(0));
      std::string fieldName = AS_STRING(readConstant())->chars;
      std::vector<std::string> struktFields = instance->strukt->fields;
      int idx = -1;
      for (int i = 0; i < struktFields.size(); ++i) {
        if (struktFields[i] == fieldName) {
          idx = i;
          break;
        }
      }
      if (idx == -1) {
        runtimeError(vm, "Couldn't find field?");
        return INTERPRET_RUNTIME_ERROR;
      }

      // Update aka remove the instance and replace it with the field
      vm->stack->update(0, instance->fields[idx]);
      break;
    }
    case OP_SET_PROPERTY: {
      if (!IS_INSTANCE(peek(1))) {
        std::cout << OBJ_TYPE(peek(1)) << "\n";
        runtimeError(vm, "Only instances have fields.");
        return INTERPRET_RUNTIME_ERROR;
      }

      ObjInstance *instance = AS_INSTANCE(peek(1));
      instance->fields[(int)readByte()] = peek(0);

      Value value = vm->stack->pop();
      vm->stack->update(0, value);
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
      if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
        concatenate();
      } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
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
      if (!IS_NUMBER(peek(0))) {
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
      if (isFalsey(peek(0))) {
        frame->ip += offset;
      }
      break;
    }
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      frame->ip -= offset;
      break;
    }
    case OP_INDEX: {
      if (!index()) {
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_CALL: {
      int argCount = readByte();
      if (!callValue(peek(argCount), argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = vm->frames[vm->frames.size() - 1];
      break;
    }
    case OP_ARRAY: {
      int argCount = readByte();
      std::vector<Value> values = std::vector<Value>();
      for (int i = 0; i < argCount; i++) {
        values.push_back(vm->stack->pop());
      }
      ObjArray *array = newArray(values);
      vm->stack->push(OBJ_VAL(array));
      break;
    }
    case OP_MAP: {
      int argCount = readByte();
      std::cout << argCount << "\n";
      std::vector<Value> values = std::vector<Value>();
      for (int i = 0; i < argCount; i++) {
        values.push_back(vm->stack->pop());
      }
      ObjMap *map = newMap(values);
      vm->stack->push(OBJ_VAL(map));
      break;
    }
    case OP_STRUCT: {
      ObjString *name = AS_STRING(readConstant());

      // This should be handled in the compiler?
      if (vm->globals.count(name->chars)) {
        runtimeError(vm, "Can't redeclare a struct '" + name->chars + "'.");
        return INTERPRET_RUNTIME_ERROR;
      }
      ObjStruct *strukt = newStruct(name);
      Value value;
      while (matchByte(OP_STRUCT_ARG)) {
        strukt->fields.push_back(AS_STRING(readConstant())->chars);
      }
      std::reverse(strukt->fields.begin(), strukt->fields.end());

      vm->globals[name->chars] = OBJ_VAL(strukt);
      break;
    }
    case OP_RETURN: {
      Value result = vm->stack->pop();
      int sp = freeFrame(vm);
      if (vm->frames.size() == 0) {
        vm->stack->pop();
        return INTERPRET_OK;
      }

      vm->stack->remove(sp);
      vm->stack->push(result);
      frame = vm->frames[vm->frames.size() - 1];
      break;
    }
    }
  }
#undef READ_STRING
#undef READ_SHORT
#undef BINARY_OP
}

InterpretResult interpret(std::string source) {
  initVM();
  Compiler *compiler = compile(source);
  if (compiler == NULL) {
    return INTERPRET_COMPILE_ERROR;
  }

  ObjFunction *function = compiler->function;
  vm->stack->push(OBJ_VAL(function));
  call(function, 0);

  freeCompiler(compiler);
  std::cout << "\n== Running in vm == \n";
  InterpretResult result = run();
  return result;
}
