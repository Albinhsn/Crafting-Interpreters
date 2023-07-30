from abc import ABC

from _token import Token


class Visitor(ABC):
    pass


class Expr(ABC):
    def accept(self, visitor: Visitor):
        pass


class Binary(Expr):
    def __init__(self, left: Expr, operator: Token, right: Expr):
        self.left: Expr = left
        self.operator: Token = operator
        self.right: Expr = right


class Grouping(Expr):
    def __init__(self, expr: Expr):
        self.expr: Expr = expr


class Literal(Expr):
    def __init__(self, value: dict):
        self.value: dict = value


class Unary(Expr):
    def __init__(self, operator: Token, right: Expr):
        self.operator: Token = operator
        self.right: Expr = right
