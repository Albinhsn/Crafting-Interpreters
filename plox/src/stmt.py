from abc import ABC
from typing import Any
from _token import Token
from expression import Expr, Visitor

class Stmt(ABC):
    def accept(self, a: Any):
        pass

class ExpressionStmt(Stmt, Visitor):
    def __init__(self,   expression: Expr):
        self.  expression: Expr =   expression


    def accept(self, visitor: Visitor):
        return visitor.visit_expression_stmt(self)
        
class PrintStmt(Stmt, Visitor):
    def __init__(self,   expression: Expr):
        self.  expression: Expr =   expression


    def accept(self, visitor: Visitor):
        return visitor.visit_print_stmt(self)
        
class VarStmt(Stmt, Visitor):
    def __init__(self,  name: Token,  initializer: Expr):
        self. name: Token =  name
        self. initializer: Expr =  initializer


    def accept(self, visitor: Visitor):
        return visitor.visit_var_stmt(self)
        
class BlockStmt(Stmt, Visitor):
    def __init__(self,  stmts: list[Stmt]):
        self.statements: list[Stmt] =  stmts


    def accept(self, visitor: Visitor):
        return visitor.visit_block_stmt(self)
        
