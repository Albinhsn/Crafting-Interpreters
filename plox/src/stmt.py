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
        return visitor.visit_expression_stmt(self)


class PrintStmt(Stmt, Visitor):
    def __init__(self, expression: Expr):
        self.expression: Expr = expression

    def accept(self, visitor: Visitor):
        return visitor.visit_print_stmt(self)


class VarStmt(Stmt, Visitor):
    def __init__(self, name: Token, initializer: Expr):
        self.name: Token = name
        self.initializer: Expr = initializer

    def accept(self, visitor: Visitor):
        return visitor.visit_var_stmt(self)


class WhileStmt(Stmt, Visitor):
    def __init__(self, condition: Expr, body: Stmt):
        self.condition: Expr = condition
        self.body: Stmt = body

    def accept(self, visitor: Visitor):
        return visitor.visit_while_stmt(self)


class BlockStmt(Stmt, Visitor):
    def __init__(self, statements: list[Stmt]):
        self.statements: list[Stmt] = statements

    def accept(self, visitor: Visitor):
        return visitor.visit_block_stmt(self)


class IfStmt(Stmt, Visitor):
    def __init__(self, condition: Expr, then_branch: Stmt, else_branch: Stmt):
        self.condition: Expr = condition
        self.then_branch: Stmt = then_branch
        self.else_branch: Stmt = else_branch

    def accept(self, visitor: Visitor):
        return visitor.visit_if_stmt(self)
