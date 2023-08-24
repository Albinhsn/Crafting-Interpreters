#ifndef cpplox_stack_h
#define cpplox_stack_h

#include "value.h"
#include <stdexcept>

typedef struct Node {
  Value value;
  Node *next;
} Node;

class Stack {
  Node *head;

public:
  int length;
  Value get(int idx) {
    Node *curr = head;
    for (int i = 0; i < idx; i++) {
      curr = curr->next;
    }
    std::cout << "Getting - " << curr->value.as.chars << "\n";
    return curr->value;
  }
  void init() {
    head = NULL;
    length = 0;
  }
  Value peek() {
    if (length == 0) {
      throw std::invalid_argument("Can't peek with 0 length");
    }
    return head->value;
  }

  Value pop() {
    if (length == 0) {
      throw std::invalid_argument("Can't pop with 0 length");
    }
    length--;
    Node oldHead = *head;
    head = head->next;
    return oldHead.value;
  }
  void push(Value value) {
    Node *node = new Node();
    node->value = value;
    node->next = head;
    head = node;
    length++;
  }
};
#endif
