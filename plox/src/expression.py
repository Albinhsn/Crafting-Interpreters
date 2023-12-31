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

    def visit_logical_expr(self, cls):
        pass

    def visit_literal_expr(self, cls):
        pass

    def visit_unary_expr(self, cls):
        pass

    def visit_set_expr(self, cls):
        pass

    def visit_super_expr(self, cls):
        pass

    def visit_this_expr(self, cls):
        pass

    def visit_variable_expr(self, cls):
        pass

    def visit_call_expr(self, cls):
        pass

    def visit_get_expr(self, cls):
        pass

    def visit_var_stmt(self, cls):
        pass

    def visit_print_stmt(self, cls):
        pass

    def visit_expression_stmt(self, cls):
        pass

    def visit_block_stmt(self, cls):
        pass

    def visit_if_stmt(self, cls):
        pass

    def visit_while_stmt(self, cls):
        pass

    def visit_function_stmt(self, cls):
        pass

    def visit_return_stmt(self, cls):
        pass

    def visit_class_stmt(self, cls):
        pass


class Expr(ABC):
    def accept(self, a: Any):
        pass


class AssignExpr(Expr, Visitor):
    def __init__(self, name: Token, value: Expr):
        self.name: Token = name
        self.value: Expr = value

    def accept(self, visitor: Visitor):
        return visitor.visit_assign_expr(self)


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


class LogicalExpr(Expr, Visitor):
    def __init__(self, left: Expr, operator: Token, right: Expr):
        self.left: Expr = left
        self.operator: Token = operator
        self.right: Expr = right

    def accept(self, visitor: Visitor):
        return visitor.visit_logical_expr(self)


class LiteralExpr(Expr, Visitor):
    def __init__(self, value: Any):
        self.value: Any = value

    def accept(self, visitor: Visitor):
        return visitor.visit_literal_expr(self)


class UnaryExpr(Expr, Visitor):
    def __init__(self, operator: Token, right: Expr):
        self.operator: Token = operator
        self.right: Expr = right

    def accept(self, visitor: Visitor):
        return visitor.visit_unary_expr(self)


class SetExpr(Expr, Visitor):
    def __init__(self, object: Expr, name: Token, value: Expr):
        self.object: Expr = object
        self.name: Token = name
        self.value: Expr = value

    def accept(self, visitor: Visitor):
        return visitor.visit_set_expr(self)


class SuperExpr(Expr, Visitor):
    def __init__(self, keyword: Token, method: Token):
        self.keyword: Token = keyword
        self.method: Token = method

    def accept(self, visitor: Visitor):
        return visitor.visit_super_expr(self)


class ThisExpr(Expr, Visitor):
    def __init__(self, keyword: Token):
        self.keyword: Token = keyword

    def accept(self, visitor: Visitor):
        return visitor.visit_this_expr(self)


class VariableExpr(Expr, Visitor):
    def __init__(self, name: Token):
        self.name: Token = name

    def accept(self, visitor: Visitor):
        return visitor.visit_variable_expr(self)


class CallExpr(Expr, Visitor):
    def __init__(self, callee: Expr, paren: Token, arguments: list[Expr]):
        self.callee: Expr = callee
        self.paren: Token = paren
        self.arguments: list[Expr] = arguments

    def accept(self, visitor: Visitor):
        return visitor.visit_call_expr(self)


class GetExpr(Expr, Visitor):
    def __init__(self, object: Expr, name: Token):
        self.object: Expr = object
        self.name: Token = name

    def accept(self, visitor: Visitor):
        return visitor.visit_get_expr(self)
