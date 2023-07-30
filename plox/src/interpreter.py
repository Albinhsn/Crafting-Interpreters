from typing import Any

from _token import Token
from expression import (BinaryExpr, Expr, GroupingExpr, LiteralExpr, UnaryExpr,
                        Visitor)
from stmt import ExpressionStmt, PrintStmt, Stmt
from token_type import TokenType


class LoxRuntimeError(Exception):
    pass


class Interpreter(Visitor):
    def __init__(self, error) -> None:
        self.error = error

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

    def visit_print_stmt(self, stmt: PrintStmt):
        value: Any = self._evaluate(stmt.expression)
        print(self._stringify(value))

    def visit_literal_expr(self, expr: LiteralExpr) -> Any:
        return expr.value

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

    def _check_number_operands(self, operator: Token, left_operand: Any, right_operand):
        if isinstance(left_operand, float) and isinstance(right_operand, float):
            return

        raise LoxRuntimeError(operator, "Operands must be a numbers")

    def _evaluate(self, expr: Expr):
        return expr.accept(self)
