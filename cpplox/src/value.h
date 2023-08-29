#ifndef cpplox_value_h
#define cpplox_value_h

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum { VAL_STRING, VAL_OBJ, VAL_BOOL, VAL_NIL, VAL_NUMBER } ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj *obj;
    const char *chars;
  } as;
} Value;

#define IS_OBJ(value) ((value).type == VAL_OBJ)
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_STRING(value) ((value).type == VAL_STRING)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)

#define AS_OBJ(value) ((value).as.obj)
#define AS_STRING(value) ((value).as.chars)
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))

#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj *)object}})
#define STRING_VAL(value) ((Value){VAL_STRING, {.chars = value}})
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

typedef std::vector<Value> ValueArray;

void printValue(Value value);
bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array, Value value);

#endif
