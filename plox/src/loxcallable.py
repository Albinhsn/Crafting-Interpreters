from __future__ import annotations

from abc import ABC
from time import time
from typing import Any, Dict, Union

from _token import Token
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
    def __init__(
        self, declaration: FunctionStmt, closure: Environment, is_initializer: bool
    ) -> None:
        self._declaration: FunctionStmt = declaration
        self.closure: Environment = closure
        self._is_initializer: bool = is_initializer
        self.logger = get_logger(__name__)

    def _call(self, interpreter, arguments: list[Any]):
        environment: Environment = Environment(self.closure)
        # self.logger.info("Calling, got new env", env=environment.enclosing.values)
        for i in range(len(self._declaration.params)):
            environment._define(self._declaration.params[i].lexeme, arguments[i]),
        try:
            interpreter._execute_block(self._declaration.body, environment)
        except Return as r:
            if self._is_initializer:
                return self.closure.get_at(0, "this")
            return r.args[0]

        if self._is_initializer:
            return self.closure.get_at(0, "this")

        return None

    def _arity(self):
        return len(self._declaration.params)

    def to_string(self):
        return f"<fn {self._declaration.name.lexeme}>"

    def bind(self, instance: LoxInstance):
        environment: Environment = Environment(self.closure)
        environment._define("this", instance)

        return LoxFunction(self._declaration, environment, self._is_initializer)


class Clock(LoxCallable):
    def __init__(self) -> None:
        pass

    def _arity(self):
        return 0

    def _call(self, interpreter, arguments):
        return time()

    def to_string(self):
        return "<native fn>"


class LoxClass(LoxCallable):
    def __init__(self, name: str, methods: Dict[str, LoxFunction]) -> None:
        self.name = name
        self.methods = methods

    def to_string(self):
        return self.name

    def _call(self, interpreter, arguments) -> Any:
        instance: LoxInstance = LoxInstance(self)

        initializer: Union[LoxFunction, None] = self.find_method("init")
        if initializer is not None:
            initializer.bind(instance)._call(interpreter, arguments)
        return instance

    def _arity(self) -> int:
        initializer: LoxFunction = self.find_method("init")

        return 0 if initializer is None else initializer._arity()

    def find_method(self, name: str) -> Union[LoxFunction, None]:
        method: Union[LoxFunction, None] = self.methods.get(name)
        return method


class LoxInstance:
    def __init__(self, klass: LoxClass) -> None:
        self.klass: LoxClass = klass
        self.fields: dict = {}

    def to_string(self) -> str:
        return self.klass.name + " instance"

    def get(self, name: Token) -> Any:
        if name.lexeme in self.fields:
            return self.fields[name.lexeme]

        method: Union[LoxFunction, None] = self.klass.find_method(name.lexeme)
        if method is not None:
            return method.bind(self)

        raise Exception(f"Undefined property {name.lexeme}")

    def set(self, name: Token, value: Any) -> None:
        self.fields[name.lexeme] = value
