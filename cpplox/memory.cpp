

#include "memory.h"
#include <cstdlib>

void *reallocate(void *pointer, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    free(pointer);
    return NULL;
  }

  void *result = std::realloc(pointer, newSize);
  return result;
}
