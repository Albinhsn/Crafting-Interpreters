from typing import Any, Optional

from _token import Token


class Environment:
    def __init__(self, enclosing = None) -> None:
        self.values: dict = {}
        self.enclosing = enclosing

    def _define(self, name: str, value: Any):
        self.values[name] = value

    def get(self, name: Token):
        if name.lexeme in self.values:
            return self.values[name.lexeme]

        if self.enclosing:
            return self.enclosing.get(name)
        raise Exception(name, "Undefined variable '" + name.lexeme + "'.")

    def _assign(self, name: Token, value: Any):
        if name.lexeme in self.values:
            values[name.lexeme] = value
            return
        if self.enclosing:
            self.enclosing._assign(name, value)
            return
        raise LoxRuntimeError("Undefied variable '" + name.lexeme + "'.")
