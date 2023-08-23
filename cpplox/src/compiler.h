#ifndef cpplox_compiler_h
#define cpplox_compiler_h

#include "common.h"
#include "vm.h"

bool compile(std::string source, Chunk *chunk);

#endif
