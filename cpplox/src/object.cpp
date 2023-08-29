
#include "object.h"

ObjFunction *newFunction() {
  ObjFunction *function = new ObjFunction();
  function->arity = 0;
  function->name = NULL;
  initChunk(function->chunk);
  return function;
}
static void printFunction(ObjFunction *function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<fn %s>", function->name->chars.c_str());
}

ObjNative *newNative(NativeFn function) {
  ObjNative *native = new ObjNative();
  native->function = function;
  return native;
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_NATIVE: {
    printf("<native fn>");
    break;
  }
  case OBJ_FUNCTION: {
    printFunction(AS_FUNCTION(value));
    break;
  }
  }
}

ObjString *copyString(std::string str) {
  ObjString *objString = new ObjString();

  Obj obj;
  obj.type = OBJ_STRING;
  objString->chars = str;

  return objString;
}
