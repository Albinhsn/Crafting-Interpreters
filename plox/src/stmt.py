from abc import ABC
from typing import Any

from _token import Token
from expression import Expr, Visitor


class Stmt(ABC):
    def accept(self, a: Any):
        pass


class ExpressionStmt(Stmt, Visitor):
    def __init__(self, expression: Expr):
        self.expression: Expr = expression

    def accept(self, visitor: Visitor):
        visitor.visit_expression_stmt(self)


class PrintStmt(Stmt, Visitor):
    def __init__(self, expression: Expr):
        self.expression: Expr = expression

    def accept(self, visitor: Visitor):
        visitor.visit_print_stmt(self)
