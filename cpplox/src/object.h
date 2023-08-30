#ifndef cpplox_object_h
#define cpplox_object_h

#include "chunk.h"
#include "common.h"
#include "value.h"

typedef enum { OBJ_STRING, OBJ_FUNCTION, OBJ_NATIVE } ObjType;

struct Obj {
  ObjType type;
};

typedef struct {
  Obj obj;
  int arity;
  Chunk *chunk;
  ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value args);

typedef struct {
  Obj obj;
  NativeFn function;
} ObjNative;

#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))

struct ObjString {
  Obj obj;
  std::string chars;
};

void printObject(Value value);
ObjFunction *newFunction();
ObjNative *newNative(NativeFn function);
ObjString *copyString(std::string str);
static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
