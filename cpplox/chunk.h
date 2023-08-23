#ifndef cpplox_chunk_h
#define cpplox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_RETURN,
} OpCode;

typedef struct {
  std::vector<uint8_t> code;
  std::vector<int> lines;
  std::vector<Value> constants;
  int count;
} Chunk;

void freeChunk(Chunk *chunk);
void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);

int addConstant(Chunk *chunk, Value value);

#endif
