
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

void printValue(Value value) { std::cout << AS_NUMBER(value); }
