

#include "debug.h"
#include "common.h"
#include "value.h"

void disassembleChunk(Chunk *chunk, std::string name) {
  std::cout << "== " << name << " ==\n";

  for (int offset = 0; offset < chunk->code.size();) {
    offset = disassembleInstruction(chunk, offset);
  }
}

static int simpleInstruction(std::string name, int offset) {
  std::cout << name << "\n";
  return offset + 1;
}
static int constantInstruction(std::string name, Chunk *chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  std::cout << name << " " << (int)constant << " '";
  printValue(chunk->constants[constant]);
  std::cout << "'\n";
  return offset + 2;
}

int disassembleInstruction(Chunk *chunk, int offset) {
  std::cout << offset << " ";
  if(offset > 0 && chunk->lines[offset] == chunk->lines[offset -1]){
    std::cout << "  | ";
  }else{
    std::cout << chunk->lines[offset] << "   ";
  }
  uint8_t instruction = chunk->code[offset];

  switch (instruction) {
  case OP_RETURN: {
    return simpleInstruction("OP_RETURN", offset);
  }
  case OP_CONSTANT: {
    return constantInstruction("OP_CONSTANT", chunk, offset);
  }
  default: {
    std::cout << "Unknown opcode" << (int)instruction << "\n";
    return offset + 1;
  }
  }
}
