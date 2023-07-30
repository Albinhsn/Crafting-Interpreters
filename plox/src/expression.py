from abc import ABC
from typing import Any

from _token import Token


class Visitor(ABC):
    pass

    def visit_binary_expr(self, cls):
        pass

    def visit_grouping_expr(self, cls):
        pass

    def visit_literal_expr(self, cls):
        pass

    def visit_unary_expr(self, cls):
        pass


class Expr(ABC):
    def accept(self, a: Any):
        pass


class BinaryExpr(Expr, Visitor):
    def __init__(self, left: Expr, operator: Token, right: Expr):
        self.left: Expr = left
        self.operator: Token = operator
        self.right: Expr = right

    def accept(self, visitor: Visitor):
        return visitor.visit_binary_expr(self)


class GroupingExpr(Expr, Visitor):
    def __init__(self, expression: Expr):
        self.expression: Expr = expression

    def accept(self, visitor: Visitor):
        return visitor.visit_grouping_expr(self)


class LiteralExpr(Expr, Visitor):
    def __init__(self, value: Any):
        self.value: dict = value

    def accept(self, visitor: Visitor):
        return visitor.visit_literal_expr(self)


class UnaryExpr(Expr, Visitor):
    def __init__(self, operator: Token, right: Expr):
        self.operator: Token = operator
        self.right: Expr = right

    def accept(self, visitor: Visitor):
        return visitor.visit_unary_expr(self)
