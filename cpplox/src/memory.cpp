

#include "memory.h"
#include <cstdlib>
#include <iostream>

void freeObjects(VM *vm) {
  for (int i = 0; i < vm->objects.size(); i++) {
    if (vm->objects[i] == NULL) {
      std::cout << "ALREADY FREED\n";
      continue;
    }
    switch (vm->objects[i]->type) {
    case OBJ_STRING: {
      ObjString *string = (ObjString *)vm->objects[i];
      delete (string);
      break;
    }
    case OBJ_NATIVE: {
      ObjNative *native = (ObjNative *)vm->objects[i];
      delete (native);
      break;
    }
    case OBJ_FUNCTION: {
      ObjFunction *fn = (ObjFunction *)vm->objects[i];
      delete (fn);
      break;
    }
    }
  }
}

void freeParser(Parser *parser) {
  if (parser->current) {

    delete (parser->current);
  }
  if (parser->previous) {
    delete (parser->previous);
  }

  delete (parser);
};

void freeScanner(Scanner * scanner){
  delete(scanner);
}

int freeFrame(VM *vm) {
  int sp = vm->frames.back()->sp;
  delete (vm->frames.back());
  vm->frames.pop_back();

  return sp;
};
void freeCompiler(Compiler *compiler) { delete (compiler); }
