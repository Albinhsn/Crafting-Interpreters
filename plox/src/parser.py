from typing import Any, Union

from _token import Token
from expression import BinaryExpr, Expr, GroupingExpr, LiteralExpr, UnaryExpr
from log import get_logger
from stmt import ExpressionStmt, PrintStmt, Stmt
from token_type import TokenType


class ParseError(Exception):
    pass


class Parser:
    def __init__(self, tokens: list[Token], error: Any):
        self._current: int = 0
        self.tokens = tokens
        self.error = error
        # self.logger = get_logger()

    def parse(self) -> list[Stmt]:
        statements: list[Stmt] = []
        while not self._is_at_end():
            statements.append(self._statement())
        return statements

    def _statement(self):
        if self._match(TokenType.PRINT):
            return self._print_statement()
        return self._expression_statement()

    def _print_statement(self) -> Stmt:
        value: Expr = self._expression()
        self._consume(TokenType.SEMICOLON, "Expect ';' after value.")

        return PrintStmt(value)

    def _expression_statement(self):
        expr: Expr = self._expression()
        self._consume(TokenType.SEMICOLON, "Expect ';' after expression")

        return ExpressionStmt(expr)

    def _expression(self) -> Expr:
        return self._equality()

    def _equality(self) -> Expr:
        expr: Expr = self._comparison()

        while self._match(TokenType.BANG_EQUAL, TokenType.EQUAL_EQUAL):
            operator: Token = self._previous()
            right: Expr = self._comparison()

            expr = BinaryExpr(expr, operator, right)

        return expr

    def _comparison(self):
        expr: Expr = self._term()

        while self._match(
            TokenType.GREATER,
            TokenType.GREATER_EQUAL,
            TokenType.LESS,
            TokenType.LESS_EQUAL,
        ):
            operator: Token = self._previous()
            right: Expr = self._term()
            expr = BinaryExpr(expr, operator, right)

        return expr

    def _term(self):
        expr: Expr = self._factor()

        while self._match(TokenType.MINUS, TokenType.PLUS):
            operator: Token = self._previous()
            right: Expr = self._factor()

            expr = BinaryExpr(expr, operator, right)

        return expr

    def _error(self, token: Token, msg: str) -> ParseError:
        self.error(token, msg)
        return ParseError()

    def _factor(self):
        expr: Expr = self._unary()

        while self._match(TokenType.SLASH, TokenType.STAR):
            operator: Token = self._previous()
            right: Expr = self._unary()

            expr = BinaryExpr(expr, operator, right)

        return expr

    def _unary(self):
        if self._match(TokenType.BANG, TokenType.MINUS):
            operator: Token = self._previous()
            right: Expr = self._unary()
            return UnaryExpr(operator, right)

        return self._primary()

    def _primary(self) -> Expr:
        if self._match(TokenType.FALSE):
            return LiteralExpr(False)

        if self._match(TokenType.TRUE):
            return LiteralExpr(True)

        if self._match(TokenType.NIL):
            return LiteralExpr(None)

        if self._match(TokenType.NUMBER, TokenType.STRING):
            return LiteralExpr(self._previous().literal)

        if self._match(TokenType.LEFT_PAREN):
            expr: Expr = self._expression()
            self._consume(TokenType.RIGHT_PAREN, "Expect ')' after expression")
            return GroupingExpr(expr)
        self._error(self._peek(), "Expect expression")

    def _consume(self, type: TokenType, msg: str):
        if self._check(type):
            return self._advance()

        self._error(self._peek(), msg)

    def _synchronize(self):
        self._advance()

        while not self._is_at_end():
            if self._previous().type == TokenType.SEMICOLON:
                return

            ttype: TokenType = self._peek().type
            match ttype:
                case TokenType.CLASS:
                    return
                case TokenType.FUN:
                    return
                case TokenType.VAR:
                    return
                case TokenType.FOR:
                    return
                case TokenType.IF:
                    return
                case TokenType.WHILE:
                    return
                case TokenType.PRINT:
                    return
                case TokenType.RETURN:
                    return
                case _:
                    pass
            self._advance()

    def _match(self, *argv: TokenType):
        for type in argv:
            if self._check(type):
                self._advance()
                return True

        return False

    def _check(self, type: TokenType):
        if self._is_at_end():
            return False
        return self._peek().type == type

    def _advance(self):
        if not self._is_at_end():
            self._current += 1
        return self._previous()

    def _is_at_end(self):
        return self._peek().type == TokenType.EOF

    def _peek(self) -> Token:
        return self.tokens[self._current]

    def _previous(self):
        return self.tokens[self._current - 1]
