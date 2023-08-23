#ifndef cpplox_chunk_h
#define cpplox_chunk_h

#include "common.h"

typedef enum {
  OP_RETURN,
} OpCode;

typedef struct {
  std::vector<uint8_t> code;
} Chunk;

void writeChunk(Chunk *chunk, uint8_t byte);
void initChunk(Chunk *chunk);

#endif
