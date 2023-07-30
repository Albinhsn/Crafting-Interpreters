from abc import ABC

from _token import Token


class Visitor(ABC):
    pass

    def visit_binary(self, cls):
        pass

    def visit_grouping(self, cls):
        pass

    def visit_literal(self, cls):
        pass

    def visit_unary(self, cls):
        pass


class Expr(ABC):
    pass


class Binary(Expr, Visitor):
    def __init__(self, left: Expr, operator: Token, right: Expr):
        self.left: Expr = left
        self.operator: Token = operator
        self.right: Expr = right

    def accept(self, visitor: Visitor):
        visitor.visit_binary(self)


class Grouping(Expr, Visitor):
    def __init__(self, expr: Expr):
        self.expr: Expr = expr

    def accept(self, visitor: Visitor):
        visitor.visit_grouping(self)


class Literal(Expr, Visitor):
    def __init__(self, value: dict):
        self.value: dict = value

    def accept(self, visitor: Visitor):
        visitor.visit_literal(self)


class Unary(Expr, Visitor):
    def __init__(self, operator: Token, right: Expr):
        self.operator: Token = operator
        self.right: Expr = right

    def accept(self, visitor: Visitor):
        visitor.visit_unary(self)
