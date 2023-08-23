#include "chunk.h"
#include "memory.h"

#define INIT_ARRRAY_SIZE 8

void initChunk(Chunk *chunk) {
  chunk->code = std::vector<uint8_t>(INIT_ARRRAY_SIZE);
}

void writeChunk(Chunk *chunk, uint8_t byte) {
  if (chunk->code.capacity() < chunk->code.size() + 1) {
    chunk->code.resize() 
  }
  chunk->code[chunk->code.size()] = byte;
}
