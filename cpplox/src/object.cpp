
#include "object.h"

ObjString *copyString(std::string str) {
  ObjString *objString = new ObjString();

  Obj obj;
  obj.type = OBJ_STRING;
  objString->chars = str;

  return objString;
}
