
#include "iostream"

#include "memory.h"
#include "object.h"
#include "value.h"
#include <cstdlib>
#include <cstring>

bool valuesEqual(Value a, Value b) {
  if (a.type != b.type)
    return false;

  switch (a.type) {
  case VAL_BOOL:
    return AS_BOOL(a) == AS_BOOL(b);
  case VAL_NIL:
    return true;
  case VAL_NUMBER:
    return AS_NUMBER(a) == AS_NUMBER(b);
  case VAL_OBJ:
    return AS_OBJ(a) == AS_OBJ(b);
  default:
    return false;
  }
}

void printValue(Value value) {
  switch (value.type) {
  case VAL_BOOL: {
    std::cout << (AS_BOOL(value) ? "true" : "false");
    break;
  }
  case VAL_NIL: {
    std::cout << "nil";
    break;
  }
  case VAL_NUMBER: {
    std::cout << AS_NUMBER(value);
    break;
  }
  case VAL_OBJ: {
    printObject(value);
    break;
  }
  default:{
      std::cout << "undefined value"; 
    }
  }
}
