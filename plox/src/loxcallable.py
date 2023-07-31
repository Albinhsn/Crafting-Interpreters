from abc import ABC
from typing import Any

from environment import Environment
from stmt import FunctionStmt


class LoxCallable(ABC):
    def _call(self, interpreter, arguments: list[Any]):
        pass

    def _arity(self) -> int:
        pass


class LoxFunction(LoxCallable):
    def __init__(self, declaration: FunctionStmt) -> None:
        self._declaration: FunctionStmt = declaration

    def _call(self, interpreter, arguments: list[Any]):
        environment: Environment = Environment(interpreter.globals)
        for i in range(len(self._declaration.params)):
            environment._define(self._declaration.params[i].lexeme, arguments[i]),

        interpreter._execute_block(self._declaration.body, environment)
        return None

    def _arity(self):
        return len(self._declaration.params)

    def to_string(self):
        return "<fn {self._declaration.name.lexeme}>"


class Clock(LoxCallable):
    def __init__(self) -> None:
        pass

    def _arity(self):
        return 0

    def _call(self, interpreter, arguments):
        return time()

    def to_string(self):
        return "<native fn>"
