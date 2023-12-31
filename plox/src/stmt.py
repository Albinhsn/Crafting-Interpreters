from __future__ import annotations

from abc import ABC
from typing import Any, Optional

from _token import Token
from expression import Expr, VariableExpr, Visitor


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


class ReturnStmt(Stmt, Visitor):
    def __init__(self, keyword: Token, value: Optional[Expr]):
        self.keyword: Token = keyword
        self.value: Optional[Expr] = value

    def accept(self, visitor: Visitor):
        return visitor.visit_return_stmt(self)


class VarStmt(Stmt, Visitor):
    def __init__(self, name: Token, initializer: Optional[Expr]):
        self.name: Token = name
        self.initializer: Optional[Expr] = initializer

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


class ClassStmt(Stmt, Visitor):
    def __init__(
        self,
        name: Token,
        methods: list[FunctionStmt],
        superclass: Optional[VariableExpr] = None,
    ):
        self.name: Token = name
        self.superclass: Optional[VariableExpr] = superclass
        self.methods: list[FunctionStmt] = methods

    def accept(self, visitor: Visitor):
        return visitor.visit_class_stmt(self)


class IfStmt(Stmt, Visitor):
    def __init__(self, condition: Expr, then_branch: Stmt, else_branch: Optional[Stmt]):
        self.condition: Expr = condition
        self.then_branch: Stmt = then_branch
        self.else_branch: Optional[Stmt] = else_branch

    def accept(self, visitor: Visitor):
        return visitor.visit_if_stmt(self)


class FunctionStmt(Stmt, Visitor):
    def __init__(self, name: Token, params: list[Token], body: list[Stmt]):
        self.name: Token = name
        self.params: list[Token] = params
        self.body: list[Stmt] = body

    def accept(self, visitor: Visitor):
        return visitor.visit_function_stmt(self)
