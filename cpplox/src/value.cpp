
#include "iostream"

#include "memory.h"
#include "value.h"
#include <cstdlib>

void initValueArray(ValueArray *array) { array = new std::vector<Value>(); }

void writeValueArray(ValueArray *array, Value value) {
  array->push_back(value);
}

void freeValueArray(ValueArray *array) {
  array->resize(0);
  initValueArray(array);
}

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
  case VAL_STRING: {
    std::cout << AS_STRING(value);
  }
  }
}
