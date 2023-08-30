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
  void remove(int sp) {
    Node *curr = head;
    while (length > sp) {
      pop();
    }
  }

  Value get(int idx) {
    Node *curr = head;
    for (int i = 0; i < idx; i++) {
      curr = curr->next;
    }
    return curr->value;
  }
  void update(int idx, Value value) {
    Node *curr = head;
    for (int i = 0; i < idx; i++) {
      curr = curr->next;
    }
    curr->value = value;
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
    Node *oldHead = head;
    head = head->next;
    Value value = oldHead->value;
    delete (oldHead);
    return value;
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
