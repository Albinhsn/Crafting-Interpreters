from typing import Any

from _token import Token
from environment import Environment
from expression import (AssignExpr, BinaryExpr, Expr, GroupingExpr,
                        LiteralExpr, LogicalExpr, UnaryExpr, VariableExpr,
                        Visitor)
from log import get_logger
from stmt import (BlockStmt, ExpressionStmt, IfStmt, PrintStmt, Stmt, VarStmt,
                  WhileStmt)
from token_type import TokenType


class LoxRuntimeError(Exception):
    pass


class Interpreter(Visitor):
    def __init__(self, error) -> None:
        self.error = error
        self.environment = Environment()
        self.logger = get_logger()

    def interpret(self, stmts: list[Stmt]):
        try:
            for stmt in stmts:
                self._execute(stmt)
        except LoxRuntimeError as e:
            self.error(e)

    def _execute(self, stmt: Stmt):
        stmt.accept(self)

    def _stringify(self, obj: Any):
        if not obj:
            return "nil"

        if isinstance(obj, float):
            txt: str = str(obj)

            if ".0" == txt[-2:]:
                txt = txt[0:-2]

            return txt

        # Does this never raise?
        return str(obj)  # should be .to_string()?

    def visit_expression_stmt(self, stmt: ExpressionStmt):
        self._evaluate(stmt.expression)

    def visit_if_stmt(self, stmt: IfStmt):
        if self._is_truthy(self._evaluate(stmt.condition)):
            self._execute(stmt.then_branch)
        elif stmt.else_branch is not None:
            self._execute((stmt.else_branch))

        return None

    def visit_block_stmt(self, stmt: BlockStmt):
        return self._execute_block(stmt.statements, Environment())

    def visit_print_stmt(self, stmt: PrintStmt):
        value: Any = self._evaluate(stmt.expression)
        print(self._stringify(value))

    def visit_var_stmt(self, stmt: VarStmt):
        value: Any = None
        if stmt.initializer is not None:
            value = self._evaluate(stmt.initializer)
        self.logger.info("Defining var stmt", value=value, name=stmt.name.lexeme)
        self.environment._define(stmt.name.lexeme, value)
        return None

    def visit_assign_expr(self, expr: AssignExpr):
        value: Any = self._evaluate(expr.value)
        self.logger.info("Assign expr", value=value)
        self.environment._assign(expr.name, value)
        return value

    def visit_literal_expr(self, expr: LiteralExpr) -> Any:
        return expr.value

    def visit_variable_expr(self, expr: VariableExpr):
        return self.environment.get(expr.name)

    def visit_logical_expr(self, expr: LogicalExpr):
        left = self._evaluate(expr.left)

        if expr.operator.type == TokenType.OR:
            if self._is_truthy(left):
                return left
        else:
            if not self._is_truthy(left):
                return left
        return self._evaluate(expr.right)

    def visit_while_stmt(self, stmt: WhileStmt):
        while self._is_truthy(self._evaluate(stmt.condition)):
            self._execute(stmt.body)

        return None

    def visit_unary_expr(self, expr: UnaryExpr):
        right: Any = self._evaluate(expr.right)

        match expr.operator.type:
            case TokenType.BANG:
                return not self._is_truthy(right)
            case TokenType.MINUS:
                self._check_number_operand(expr.operator, right)
                return -float(right)
            case _:
                pass

        return None

    def _is_truthy(self, obj: Any):
        if not obj:
            return False

        if isinstance(obj, bool):
            return bool(obj)

        return True

    def visit_binary_expr(self, expr: BinaryExpr):
        left: Any = self._evaluate(expr.left)
        right: Any = self._evaluate(expr.right)

        match expr.operator.type:
            case TokenType.BANG_EQUAL:
                return left != right

            case TokenType.EQUAL:
                return left == right

            case TokenType.GREATER:
                self._check_number_operands(expr.operator, left, right)
                return left > right

            case TokenType.GREATER_EQUAL:
                self._check_number_operands(expr.operator, left, right)
                return left >= right

            case TokenType.LESS:
                self._check_number_operands(expr.operator, left, right)
                return left < right

            case TokenType.LESS_EQUAL:
                self._check_number_operands(expr.operator, left, right)
                return left <= right

            case TokenType.MINUS:
                return float(left) - float(right)

            case TokenType.PLUS:
                if isinstance(left, float) and isinstance(right, float):
                    return left + right

                if isinstance(left, str) and isinstance(right, str):
                    return left + right
                raise Exception(f"Trying to + smth? {type(left)} and {type(right)}")

            case TokenType.SLASH:
                self._check_number_operands(expr.operator, left, right)
                return float(left) / float(right)

            case TokenType.STAR:
                self._check_number_operands(expr.operator, left, right)
                return float(left) * float(right)

        return None

    def visit_grouping_expr(self, expr: GroupingExpr):
        return self._evaluate(expr)

    def _check_number_operand(self, operator: Token, operand: Any):
        if isinstance(operand, float):
            return

        raise LoxRuntimeError(operator, "Operand must be a number")

    def _execute_block(self, statements: list[Stmt], environment: Environment):
        previous: Environment = self.environment
        try:
            self.environment = environment
            self.logger.info("Environment", env=self.environment.values, arg=environment.values)
            for statement in statements:
                self._execute(statement)
        finally:
            self.environment = previous

    def _check_number_operands(self, operator: Token, left_operand: Any, right_operand):
        if isinstance(left_operand, float) and isinstance(right_operand, float):
            return

        raise LoxRuntimeError(operator, "Operands must be a numbers")

    def _evaluate(self, expr: Expr):
        return expr.accept(self)
