#ifndef cpplox_object_h
#define cpplox_object_h

#include "chunk.h"
#include "common.h"
#include "value.h"
#include <map>

typedef enum {
  OBJ_MAP,
  OBJ_INSTANCE,
  OBJ_STRUCT,
  OBJ_STRING,
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_ARRAY
} ObjType;

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

typedef struct {
  Obj obj;
  ObjString *name;
  std::vector<std::string> fields;
} ObjStruct;

typedef struct {
  Obj obj;
  std::vector<Value> values;
} ObjArray;

typedef struct {
  Obj obj;
  ObjString *name;
  ObjStruct *strukt;
  std::vector<Value> fields;
} ObjInstance;

typedef struct {
  Obj obj;
  std::map<std::string, Value> m;
} ObjMap;

#define IS_STRUCT(value) isObjType(value, OBJ_STRUCT)
#define IS_ARRAY(value) isObjType(value, OBJ_ARRAY)
#define IS_MAP(value) isObjType(value, OBJ_MAP)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define AS_ARRAY(value) ((ObjArray *)AS_OBJ(value))
#define AS_MAP(value) ((ObjMap *)AS_OBJ(value))
#define AS_STRUCT(value) ((ObjStruct *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))

struct ObjString {
  Obj obj;
  std::string chars;
};

void printObject(Value value);
ObjFunction *newFunction();
ObjMap *newMap(std::vector<Value>);
ObjArray *newArray(std::vector<Value>);
ObjNative *newNative(NativeFn function);
ObjString *copyString(std::string str);
ObjStruct *newStruct(ObjString *name);
ObjInstance *newInstance(ObjStruct *strukt, std::vector<Value> fields);
static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
