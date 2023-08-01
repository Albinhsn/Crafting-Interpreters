import enum
from typing import Optional, Union

from _token import Token
from expression import (AssignExpr, BinaryExpr, CallExpr, Expr, GetExpr,
                        GroupingExpr, LiteralExpr, LogicalExpr, SetExpr,
                        SuperExpr, ThisExpr, UnaryExpr, VariableExpr, Visitor)
from log import get_logger
from stack import Stack
from stmt import (BlockStmt, ClassStmt, ExpressionStmt, FunctionStmt, IfStmt,
                  PrintStmt, ReturnStmt, Stmt, VarStmt, WhileStmt)


class FunctionType(enum.Enum):
    NONE = 1
    FUNCTION = 2
    INITIALIZER = 4
    METHOD = 3


class ClassType(enum.Enum):
    NONE = 1
    CLASS = 2
    SUBCLASS = 3


class Resolver(Visitor):
    def __init__(self, interpreter) -> None:
        self.interpreter = interpreter
        self.scopes = Stack()
        self.current_function = FunctionType.NONE
        self.error = interpreter.error
        self.logger = get_logger()
        self.current_class = ClassType.NONE

    def resolve(self, res: Union[list[Stmt], Stmt, Expr]) -> None:
        if isinstance(res, list):
            for stmt in res:
                self.resolve(stmt)
        elif isinstance(res, Stmt) or isinstance(res, Expr):
            res.accept(self)
        else:
            raise Exception(f"Can't resolve type {type(res)}")

    def begin_scope(self) -> None:
        self.scopes.push({})

    def end_scope(self) -> None:
        self.scopes.pop()

    def visit_block_stmt(self, stmt: BlockStmt) -> None:
        self.begin_scope()
        self.resolve(stmt.statements)
        self.end_scope()

    def visit_class_stmt(self, stmt: ClassStmt) -> None:
        enclosing_class: ClassType = self.current_class
        self.current_class = ClassType.CLASS

        self.declare(stmt.name)
        self.define(stmt.name)

        if stmt.superclass and stmt.superclass.name.lexeme == stmt.name.lexeme:
            self.error(stmt.superclass.name.lexeme, "A class can't inherit from itself")

        if stmt.superclass:
            self.current_class = ClassType.SUBCLASS
            self.resolve(stmt.superclass)

        if stmt.superclass:
            self.begin_scope()
            self.scopes.peek()["super"] = True

        self.begin_scope()  # Creates the scope for below
        self.scopes.peek()["this"] = True

        for method in stmt.methods:
            declaration: FunctionType = FunctionType.METHOD
            if method.name.lexeme == "init":
                declaration = FunctionType.INITIALIZER

            self.resolve_function(method, declaration)
        self.end_scope()

        if stmt.superclass:
            self.end_scope()

        self.current_class = enclosing_class

    def visit_var_stmt(self, stmt: VarStmt) -> None:
        self.declare(stmt.name)

        if stmt.initializer:
            self.resolve(stmt.initializer)

        self.define(stmt.name)

    def visit_expression_stmt(self, stmt: ExpressionStmt) -> None:
        self.resolve(stmt.expression)

    def visit_if_stmt(self, stmt: IfStmt) -> None:
        self.resolve(stmt.condition)
        self.resolve(stmt.then_branch)
        if stmt.else_branch:
            self.resolve(stmt.else_branch)

    def visit_function_stmt(self, stmt: FunctionStmt) -> None:
        self.declare(stmt.name)
        self.define(stmt.name)

        self.resolve_function(stmt, FunctionType.FUNCTION)

    def visit_print_stmt(self, stmt: PrintStmt) -> None:
        self.resolve(stmt.expression)

    def visit_return_stmt(self, stmt: ReturnStmt) -> None:
        if self.current_function == FunctionType.NONE:
            self.error(stmt.keyword, "Can't return from top-level code")

        if stmt.value:
            if self.current_function == FunctionType.INITIALIZER:
                self.error(stmt.keyword, "Can't return a value from an initializer")

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

    def visit_get_expr(self, expr: GetExpr) -> None:
        self.resolve(expr.object)

    def visit_grouping_expr(self, expr: GroupingExpr) -> None:
        self.resolve(expr.expression)

    def visit_literal_expr(self, expr: LiteralExpr) -> None:
        ...

    def visit_logical_expr(self, expr: LogicalExpr) -> None:
        self.resolve(expr.left)
        self.resolve(expr.right)

    def visit_set_expr(self, expr: SetExpr) -> None:
        self.resolve(expr.value)
        self.resolve(expr.object)

    def visit_super_expr(self, expr: SuperExpr) -> None:
        if self.current_class == ClassType.NONE:
            self.error(expr.keyword, "Can't use 'super' outside of a class")
        elif self.current_class != ClassType.SUBCLASS:
            self.error(expr.keyword, "Can't use 'super' in a class with no superclass")

        self.resolve_local(expr, expr.keyword)

    def visit_this_expr(self, expr: ThisExpr) -> None:
        if self.current_class == ClassType.NONE:
            self.error(expr.keyword, "Can't use 'this' outside of a class")
            return

        self.resolve_local(expr, expr.keyword)

    def visit_unary_expr(self, expr: UnaryExpr) -> None:
        self.resolve(expr.right)

    def visit_variable_expr(self, expr: VariableExpr) -> None:
        if (
            self.scopes.length != 0
            and self.scopes.peek().get(expr.name.lexeme) is False
        ):
            self.interpreter.error(
                expr.name, "Can't read local variable in its own initializer"
            )

        self.resolve_local(expr, expr.name)

    def visit_assign_expr(self, expr: AssignExpr) -> None:
        self.resolve(expr.value)
        self.resolve_local(expr, expr.name)

    def declare(self, name: Token) -> None:
        if self.scopes.length == 0:
            return
        scope: dict = self.scopes.peek()
        if name.lexeme in scope:
            self.error(name, "Already a variable with this name in this scope")
        scope[name.lexeme] = False

    def resolve_local(self, expr: Expr, name: Token) -> None:
        self.logger.info("Resolving local", name=name.lexeme, peek=self.scopes.peek())
        for i in range(self.scopes.length - 1, -1, -1):
            value: Optional[dict] = self.scopes.get(i)
            if value and name.lexeme in value:
                self.interpreter.resolve(expr, self.scopes.length - 1 - i)
                return

    def define(self, name: Token) -> None:
        scope = self.scopes.peek()
        if scope is None:
            return

        scope[name.lexeme] = True

    def resolve_function(self, function: FunctionStmt, func_type: FunctionType):
        enclosing_function: FunctionType = self.current_function
        self.current_function = func_type

        self.begin_scope()
        for param in function.params:
            self.declare(param)
            self.define(param)
        self.resolve(function.body)
        self.end_scope()
        self.current_function = enclosing_function
