
#include "object.h"
#include "value.h"
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
ObjMap *newMap(std::vector<Value> values) {
  ObjMap *mp = new ObjMap();

  Obj obj;
  obj.type = OBJ_MAP;
  mp->obj = obj;

  mp->m = std::map<std::string, Value>();
  for (int i = 0; i < values.size(); i += 2) {
    mp->m[AS_STRING(values[i + 1])->chars] = values[i];
  }
  return mp;
}

ObjArray *newArray(std::vector<Value> values) {
  ObjArray *array = new ObjArray();
  array->values = values;

  Obj obj;
  obj.type = OBJ_ARRAY;
  array->obj = obj;

  return array;
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
  case OBJ_ARRAY: {
    ObjArray *array = AS_ARRAY(value);
    std::cout << "[";
    for (int i = 0; i < array->values.size(); i++) {
      printValue(array->values[i]);
      std::cout << (i < array->values.size() - 1 ? "," : "");
    }
    std::cout << "]";
    break;
  }
  case OBJ_MAP: {
    ObjMap *mp = AS_MAP(value);
    std::cout << "{";
    int i = 0;
    for (const auto &[key, value] : mp->m) {
      std::cout << "'" << key << "':";
      printValue(value);
      std::cout << (i < mp->m.size() - 1 ? "," : "");
      i++;
    }
    std::cout << "}";
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
