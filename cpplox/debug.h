#ifndef cpplox_debug_h
#define cpplox_debug_h

#include "chunk.h"
#include <string>

void disassembleChunk(Chunk *chunk, std::string name);
int disassembleInstruction(Chunk *chunk, int offset);

#endif
