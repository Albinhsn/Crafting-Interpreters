from __future__ import annotations

from typing import Any, Optional

from _token import Token
from log import get_logger


class Environment:
    def __init__(self, enclosing=None) -> None:
        self.values: dict = {}
        self.enclosing: Optional[Environment] = enclosing
        self.logger = get_logger()

    def _define(self, name: str, value: Any) -> None:
        self.values[name] = value

    def get(self, name: Token):
        if name.lexeme in self.values:
            return self.values[name.lexeme]

        if self.enclosing:
            return self.enclosing.get(name)
        raise Exception(name, "Undefined variable '" + name.lexeme + "'.")

    def get_at(self, distance: int, name: str) -> Any:
        return self.__ancestor(distance).values.get(name)

    def ancestor(self, distance: int) -> Environment:
        environment: Environment = self
        for _ in range(distance):
            enclosing = environment.enclosing
            if not enclosing:
                raise Exception("No ancestor")
            environment = enclosing

        return environment

    def _assign(self, name: Token, value: Any):
        if name.lexeme in self.values:
            self.values[name.lexeme] = value
            return
        if self.enclosing:
            self.enclosing._assign(name, value)
            return
        raise Exception("Undefined variable '" + name.lexeme + "'.")

    def __assign_at(self, distance: int, name: Token, value: Any) -> None:
        self.ancestor(distance).values[name.lexeme] = value
