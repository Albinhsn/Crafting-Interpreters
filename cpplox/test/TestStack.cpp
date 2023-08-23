
#include "../src/stack.h"
#include <gtest/gtest.h>

TEST(TestStack, TestStack) {
  Stack *stack = new Stack();
  stack->init();
  stack->length = 0;

  Value *v1 = new Value(5.0);
  stack->push(v1);

  Value *v2 = new Value(7.0);
  stack->push(v2);

  Value *v3 = new Value(9.0);
  stack->push(v3);
  EXPECT_EQ(stack->length, 3);

  EXPECT_EQ(stack->pop(), 9.0);
  EXPECT_EQ(stack->length, 2);

  Value *v4 = new Value(11.0);
  stack->push(v4);
  EXPECT_EQ(stack->pop(), 11.0);

  EXPECT_EQ(stack->pop(), 7.0);
  EXPECT_EQ(*stack->peek(), 5.0);
  EXPECT_EQ(stack->pop(), 5.0);
  std::cout << stack->length << "\n";
  EXPECT_EQ(stack->pop(), NULL);

  Value *v5 = new Value(69.0);
  stack->push(v5);
  EXPECT_EQ(*stack->peek(), 69.0);
  EXPECT_EQ(stack->length, 1);
}
