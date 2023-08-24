#ifndef cpplox_object_h
#define cpplox_object_h

#include "common.h"

#include "value.h"

typedef enum {
  OBJ_STRING,
} ObjType;

struct Obj {
  ObjType type;
};

struct ObjString {
  Obj obj;
  std::string chars;
};

ObjString *copyString(std::string str);


#endif
