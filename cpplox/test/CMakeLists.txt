cmake_minimum_required(VERSION 3.14)
set(This cpploxTests)

set(test_SOURCES
  ./TestStack.cpp
  ../src/stack.h
  )
add_executable(${This} ${test_SOURCES})

target_link_libraries(${This} PUBLIC
  gtest_main
  cpplox
)

include(GoogleTest)

add_test(
  NAME ${This}
  COMMAND ${This}
  )
