from typing import Any, Optional

from _token import Token
from log import get_logger


class Environment:
    def __init__(self, enclosing=None) -> None:
        self.values: dict = {}
        self.enclosing = enclosing
        self.logger = get_logger()

    def _define(self, name: str, value: Any):
        self.logger.info("Defining in env", name=name, value=value)
        self.values[name] = value

    def get(self, name: Token):
        if name.lexeme in self.values:
            return self.values[name.lexeme]

        if self.enclosing:
            return self.enclosing.get(name)
        raise Exception(name, "Undefined variable '" + name.lexeme + "'.")

    def _assign(self, name: Token, value: Any):
        if name.lexeme in self.values:
            self.values[name.lexeme] = value
            return
        if self.enclosing:
            self.enclosing._assign(name, value)
            return
        raise Exception("Undefined variable '" + name.lexeme + "'.")
