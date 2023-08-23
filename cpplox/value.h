#ifndef cpplox_value_h
#define cpplox_value_h

#include "common.h"

typedef double Value;

typedef std::vector<Value> ValueArray;

void printValue(Value value);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array, Value value);

#endif
