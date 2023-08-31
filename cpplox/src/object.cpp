
#include "object.h"
#include "vm.h"

ObjFunction *newFunction() {
  ObjFunction *function = new ObjFunction();
  function->arity = 0;
  function->name = NULL;
  function->chunk = new Chunk();
  function->obj.type = OBJ_FUNCTION;
  initChunk(function->chunk);

  vm->objects.push_back((Obj *)function);
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

  vm->objects.push_back((Obj *)function);

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
  case OBJ_STRING: {
    std::cout << AS_STRING(value)->chars;
    break;
  }
  default: {
    std::cout << OBJ_TYPE(value) << " is unknown";
  }
  }
}

ObjString *copyString(std::string str) {
  ObjString *objString = new ObjString();

  Obj *obj = new Obj();
  obj->type = OBJ_STRING;
  objString->obj = *obj;
  objString->chars = str;

  vm->objects.push_back((Obj *)objString);
  vm->objects.push_back((Obj *)obj);

  return objString;
}
