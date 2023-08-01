from abc import ABC
from time import time
from typing import Any

from environment import Environment
from log import get_logger
from stmt import FunctionStmt


class Return(Exception):
    pass


class LoxCallable(ABC):
    def _call(self, interpreter, arguments: list[Any]):
        pass

    def _arity(self) -> int:
        pass


class LoxFunction(LoxCallable):
    def __init__(self, declaration: FunctionStmt, closure: Environment) -> None:
        self._declaration: FunctionStmt = declaration
        self.closure: Environment = closure
        self.logger = get_logger(__name__)

    def _call(self, interpreter, arguments: list[Any]):
        environment: Environment = Environment(self.closure)
        # self.logger.info("Calling, got new env", env=environment.enclosing.values)
        for i in range(len(self._declaration.params)):
            environment._define(self._declaration.params[i].lexeme, arguments[i]),
        try:
            interpreter._execute_block(self._declaration.body, environment)
        except Return as r:
            return r.args[0]

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
