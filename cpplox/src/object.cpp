
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

ObjStruct *newStruct(ObjString *name) {
  ObjStruct *strukt = new ObjStruct();
  strukt->obj.type = OBJ_STRUCT;
  strukt->fields = std::vector<std::string>();
  strukt->name = name;

  vm->objects.push_back((Obj *)strukt);
  return strukt;
}

ObjInstance *newInstance(ObjStruct *strukt, std::vector<Value> fields) {
  ObjInstance *instance = new ObjInstance();
  instance->name = strukt->name;
  instance->fields = fields;
  instance->strukt = strukt;
  instance->obj.type = OBJ_INSTANCE;

  vm->objects.push_back((Obj *)instance);
  return instance;
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
  case OBJ_STRUCT: {
    std::cout << AS_STRUCT(value)->name->chars << " struct";
    break;
  }
  case OBJ_INSTANCE: {
    std::cout << AS_INSTANCE(value)->name->chars << " instance";
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
