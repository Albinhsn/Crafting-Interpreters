from __future__ import annotations

from typing import Any, Optional

from _token import Token
from log import get_logger


class Environment:
    def __init__(self, enclosing=None) -> None:
        self.values: dict = {}
        self.enclosing: Optional[Environment] = enclosing
        self.logger = get_logger()

    def get(self, name: Token):
        if name.lexeme in self.values:
            return self.values[name.lexeme]

        if self.enclosing:
            return self.enclosing.get(name)
        raise Exception(name, "Undefined variable '" + name.lexeme + "'.")

    def assign(self, name: Token, value: Any):
        if name.lexeme in self.values:
            self.values[name.lexeme] = value
            return
        if self.enclosing:
            self.enclosing.assign(name, value)
            return
        raise Exception("Undefined variable '" + name.lexeme + "'.")

    def define(self, name: str, value: Any) -> None:
        self.values[name] = value

    def ancestor(self, distance: int) -> Environment:
        environment: Environment = self
        self.logger.info("Looking for ancestor", distance=distance, env=self.values)
        for _ in range(distance):
            if environment.enclosing is None:
                raise Exception("No ancestor")
            environment = environment.enclosing

        return environment

    def get_at(self, distance: int, name: str) -> Any:
        return self.ancestor(distance).values.get(name)

    def assign_at(self, distance: int, name: Token, value: Any) -> None:
        self.ancestor(distance).values[name.lexeme] = value

    def to_string(self):
        result = str(self.values)
        if self.enclosing:
            result += " -> " + self.enclosing.to_string()

        return result

