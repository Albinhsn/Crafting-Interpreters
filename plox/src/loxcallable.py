from __future__ import annotations

from abc import ABC
from time import time
from typing import Any, Dict, Optional, Union

from _token import Token
from environment import Environment
from log import get_logger
from stmt import FunctionStmt


class Return(Exception):
    pass


class LoxCallable(ABC):
    def call(*args, **kwargs):
        ...

    def arity(*args, **kwargs):
        ...


class LoxFunction(LoxCallable):
    def __init__(
        self, declaration: FunctionStmt, closure: Environment, is_initializer: bool
    ) -> None:
        self.declaration: FunctionStmt = declaration
        self.closure: Environment = closure
        self.is_initializer: bool = is_initializer
        self.logger = get_logger(__name__)

    def bind(self, instance: LoxInstance):
        environment: Environment = self.closure
        environment.define("this", instance)

        return LoxFunction(self.declaration, environment, self.is_initializer)

    def to_string(self):
        return f"<fn {self.declaration.name.lexeme}>"

    def arity(self):
        return len(self.declaration.params)

    def call(self, interpreter, arguments: list[Any]):
        environment: Environment = self.closure

        for i in range(len(self.declaration.params)):
            environment.define(self.declaration.params[i].lexeme, arguments[i])

        try:
            interpreter.execute_block(self.declaration.body, environment)
        except Return as r:
            if self.is_initializer:
                return self.closure.get_at(0, "this")
            return r.args[0]

        if self.is_initializer:
            return self.closure.get_at(0, "this")


class Clock(LoxCallable):
    def __init__(self) -> None:
        pass

    def arity(self):
        return 0

    def call(self, interpreter, arguments):
        return time()

    def to_string(self):
        return "<native fn>"


class LoxClass(LoxCallable):
    def __init__(
        self, name: str, superclass: Optional[LoxClass], methods: Dict[str, LoxFunction]
    ) -> None:
        self.name: str = name
        self.methods: Dict[str, LoxFunction] = methods
        self.superclass: Optional[LoxClass] = superclass

    def to_string(self):
        return self.name

    def call(self, interpreter, arguments) -> Any:
        instance: LoxInstance = LoxInstance(self)

        initializer: Union[LoxFunction, None] = self.find_method("init")
        if initializer is not None:
            initializer.bind(instance).call(interpreter, arguments)

        return instance

    def arity(self) -> int:
        initializer: Union[LoxFunction, None] = self.find_method("init")
        return 0 if initializer is None else initializer.arity()

    def find_method(self, name: str) -> Union[LoxFunction, None]:
        if name in self.methods:
            return self.methods[name]

        if self.superclass is not None:
            return self.superclass.find_method(name)


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
