import enum
from typing import Union

from _token import Token
from expression import (AssignExpr, BinaryExpr, CallExpr, Expr, GroupingExpr,
                        LiteralExpr, LogicalExpr, UnaryExpr, VariableExpr,
                        Visitor)
from stack import Stack
from stmt import (BlockStmt, ClassStmt, ExpressionStmt, FunctionStmt, IfStmt,
                  PrintStmt, ReturnStmt, Stmt, VarStmt, WhileStmt)


class FunctionType(enum.Enum):
    NONE = 1
    FUNCTION = 2


class Resolver(Visitor):
    def __init__(self, interpreter) -> None:
        self.interpreter = interpreter
        self.scopes = Stack()
        self._current_function = FunctionType.NONE
        self.error = interpreter.error

    def resolve(self, res: Union[list[Stmt], Stmt, Expr]) -> None:
        if isinstance(res, list):
            for stmt in res:
                self.resolve(stmt)

        elif isinstance(res, Stmt):
            res.accept(self)

        elif isinstance(res, Expr):
            res.accept(self)
        else:
            raise Exception(f"Can't resolve type {type(res)}")

    def __begin_scope(self) -> None:
        self.scopes.push({})

    def __end_scope(self) -> None:
        self.scopes.pop()

    def visit_block_stmt(self, stmt: BlockStmt) -> None:
        self.__begin_scope()
        self.resolve(stmt.statements)
        self.__end_scope()

    def visit_class_stmt(self, stmt: ClassStmt) -> None:
        self.__declare(stmt.name)
        self.__define(stmt.name)

    def visit_var_stmt(self, stmt: VarStmt) -> None:
        self.__declare(stmt.name)
        if stmt.initializer is not None:
            self.resolve(stmt.initializer)
        self.__define(stmt.name)

    def visit_expression_stmt(self, stmt: ExpressionStmt) -> None:
        self.resolve(stmt.expression)

    def visit_if_stmt(self, stmt: IfStmt) -> None:
        self.resolve(stmt.condition)
        self.resolve(stmt.then_branch)
        if stmt.else_branch:
            self.resolve(stmt.else_branch)

    def visit_function_stmt(self, stmt: FunctionStmt) -> None:
        self.__declare(stmt.name)
        self.__define(stmt.name)

        self.__resolve_function(stmt, FunctionType.FUNCTION)

    def visit_print_stmt(self, stmt: PrintStmt) -> None:
        self.resolve(stmt.expression)

    def visit_return_stmt(self, stmt: ReturnStmt) -> None:
        if self._current_function == FunctionType.NONE:
            self.error(stmt.keyword, "Can't return from top-level code")
        if stmt.value:
            self.resolve(stmt.value)

    def visit_while_stmt(self, stmt: WhileStmt) -> None:
        self.resolve(stmt.condition)
        self.resolve(stmt.body)

    def visit_binary_expr(self, expr: BinaryExpr) -> None:
        self.resolve(expr.left)
        self.resolve(expr.right)

    def visit_call_expr(self, expr: CallExpr) -> None:
        self.resolve(expr.callee)

        for argument in expr.arguments:
            self.resolve(argument)

    def visit_grouping_expr(self, expr: GroupingExpr) -> None:
        self.resolve(expr.expression)

    def visit_literal_expr(self, expr: LiteralExpr) -> None:
        ...

    def visit_logical_expr(self, expr: LogicalExpr) -> None:
        self.resolve(expr.left)
        self.resolve(expr.right)

    def visit_unary_expr(self, expr: UnaryExpr) -> None:
        self.resolve(expr.right)

    def visit_variable_expr(self, expr: VariableExpr) -> None:
        if self.scopes.length != 0 and not self.scopes.peek()[expr.name.lexeme]:
            self.interpreter.error(
                expr.name, "Can't read local variable in its own initializer"
            )

        self.__resolve_local(expr, expr.name)

    def visit_assign_expr(self, expr: AssignExpr) -> None:
        self.resolve(expr.value)
        self.__resolve_local(expr, expr.name)

    def __declare(self, name: Token) -> None:
        if self.scopes.length == 0:
            return
        scope: dict = self.scopes.peek()
        if name.lexeme in scope:
            self.error(name, "Already a variable with this name in this scope")
        scope[name.lexeme] = False

    def __resolve_local(self, expr: Expr, name: Token) -> None:
        for i in range(self.scopes.length - 1, -1, -1):
            node = self.scopes.get(i)
            if not node:
                raise Exception("No node in get?")
            if name.lexeme in node.value:
                self.interpreter.__resolve(expr, self.scopes.length - 1 - i)
                return

    def __define(self, name: Token) -> None:
        if self.scopes.length == 0:
            return

        self.scopes.peek()[name] = True

    def __resolve_function(self, function: FunctionStmt, func_type: FunctionType):
        enclosing_function: FunctionType = self._current_function
        self._current_function = func_type

        self.__begin_scope()
        for param in function.params:
            self.__declare(param)
            self.__define(param)

        self.resolve(function.body)
        self.__end_scope()
        self._current_function = enclosing_function
