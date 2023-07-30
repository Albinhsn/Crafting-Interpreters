from abc import ABC
from typing import Any
from _token import Token
class Visitor(ABC):
    pass

    def visit_assign_expr(self, cls):
        pass

    def visit_binary_expr(self, cls):
        pass

    def visit_grouping_expr(self, cls):
        pass

    def visit_literal_expr(self, cls):
        pass

    def visit_unary_expr(self, cls):
        pass

    def visit_variable_expr(self, cls):
        pass

    def visit_var_stmt(self, cls):
        pass

    def visit_print_stmt(self, cls):
        pass

    def visit_expression_stmt(self, cls):
        pass

    def visit_block_stmt(self, cls):
        pass
class Expr(ABC):
    def accept(self, a: Any):
        pass

class AssignExpr(Expr, Visitor):
    def __init__(self,  name: Token,  value: Expr):
        self. name: Token =  name
        self. value: Expr =  value


    def accept(self, visitor: Visitor):
        return visitor.visit_assign_expr(self)
        
class BinaryExpr(Expr, Visitor):
    def __init__(self, left: Expr,  operator: Token,  right: Expr):
        self.left: Expr = left
        self. operator: Token =  operator
        self. right: Expr =  right


    def accept(self, visitor: Visitor):
        return visitor.visit_binary_expr(self)
        
class GroupingExpr(Expr, Visitor):
    def __init__(self, expression: Expr):
        self.expression: Expr = expression


    def accept(self, visitor: Visitor):
        return visitor.visit_grouping_expr(self)
        
class LiteralExpr(Expr, Visitor):
    def __init__(self, value: dict):
        self.value: dict = value


    def accept(self, visitor: Visitor):
        return visitor.visit_literal_expr(self)
        
class UnaryExpr(Expr, Visitor):
    def __init__(self, operator: Token, right:Expr):
        self.operator: Token = operator
        self.right:Expr = right


    def accept(self, visitor: Visitor):
        return visitor.visit_unary_expr(self)
        
class VariableExpr(Expr, Visitor):
    def __init__(self,  name: Token):
        self. name: Token =  name


    def accept(self, visitor: Visitor):
        return visitor.visit_variable_expr(self)
        