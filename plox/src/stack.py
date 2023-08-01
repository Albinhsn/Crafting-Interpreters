from __future__ import annotations

from typing import Any, Optional, Union


class Node:
    def __init__(self, value: Any, prev: Optional[Node] = None) -> None:
        self.prev = prev
        self.value = value


class Stack:
    def __init__(self) -> None:
        self.length: int = 0
        self.head: Optional[Node] = None

    def peek(self):
        return self.head.value if self.head else None

    def push(self, item: Any):
        self.length += 1
        node: Node = Node(value=item)
        if not self.head:
            self.head = node
            return

        node.prev = self.head
        self.head = node

    def pop(self):
        if self.head is None:
            return None
        self.length -= 1
        out: Any = self.head.value
        if self.length == 0:
            self.head = None
        else:
            self.head = self.head.prev

        return out

    def get(self, idx: int) -> Union[Any, None]:
        if not self.head:
            return None
        node: Node = self.head
        i = 0
        while i != idx:
            node = self.head.prev
            if not node and i != idx:
                return None
            i += 1

        return node.value
