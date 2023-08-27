#ifndef cpplox_chunk_h
#define cpplox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_PRINT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_DEFINE_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_NEGATE,
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
