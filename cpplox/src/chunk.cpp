#include "chunk.h"
#include <cstdlib>

void initChunk(Chunk *chunk) {
  chunk->code = std::vector<uint8_t>();
  chunk->constants = std::vector<Value>();
  chunk->lines = std::vector<int>();
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
  chunk->code.push_back(byte);
  chunk->lines.push_back(line);
}

void freeChunk(Chunk *chunk) {
  chunk->code.resize(0);
  chunk->constants.resize(0);
  chunk->lines.resize(0);
  initChunk(chunk);
}

int addConstant(Chunk *chunk, Value value) {
  chunk->constants.push_back(value);
  return chunk->constants.size() - 1;
}
