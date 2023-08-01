import time
from typing import Any, Union

from _token import Token
from expression import (AssignExpr, BinaryExpr, CallExpr, Expr, GetExpr,
                        GroupingExpr, LiteralExpr, LogicalExpr, SetExpr,
                        SuperExpr, ThisExpr, UnaryExpr, VariableExpr)
from log import get_logger
from stmt import (BlockStmt, ClassStmt, ExpressionStmt, FunctionStmt, IfStmt,
                  PrintStmt, ReturnStmt, Stmt, VarStmt, WhileStmt)
from token_type import TokenType


class ParseError(Exception):
    pass


class Parser:
    def __init__(self, tokens: list[Token], error: Any):
        self._current: int = 0
        self.tokens = tokens
        self.error = error
        self.logger = get_logger()

    def parse(self) -> list[Stmt]:
        statements: list[Stmt] = []
        while not self._is_at_end():
            statements.append(self._declaration())
        # self.logger.info("Got statements", len=len(statements))
        return statements

    def _declaration(self) -> Stmt:
        try:
            # self.logger.info("Declaration,trying to find", type=self.tokens[self._current].type)
            if self._match(TokenType.CLASS):
                return self.__class_declaration()
            if self._match(TokenType.FUN):
                return self._function("function")
            if self._match(TokenType.VAR):
                return self._var_declaration()
            return self._statement()
        except ParseError:
            self._synchronize()
            return None

    def __class_declaration(self) -> Stmt:
        name: Token = self._consume(TokenType.IDENTIFIER, "Expect class name.")

        superclass = None
        if self._match(TokenType.LESS):
            self._consume(TokenType.IDENTIFIER, "Expect superclass name")
            superclass = VariableExpr(self._previous())

        self._consume(TokenType.LEFT_BRACE, "Expect '{' before class body")
        methods: list[Stmt] = []
        while not self._check(TokenType.RIGHT_BRACE) and not self._is_at_end():
            methods.append(self._function("method"))

        self._consume(TokenType.RIGHT_BRACE, "Expect '}' after class body")
        # self.logger.info("Returning class", name=name.lexeme)
        return ClassStmt(name, superclass, methods)

    def _function(self, kind: str):
        name: Token = self._consume(TokenType.IDENTIFIER, "Expect " + kind + " name.")
        self._consume(TokenType.LEFT_PAREN, f"Expect '(' after {kind} name")

        parameters: list[Token] = []

        if not self._check(TokenType.RIGHT_PAREN):
            # Lords loop
            flag = True
            while flag or self._match(TokenType.COMMA):
                parameters.append(
                    self._consume(TokenType.IDENTIFIER, "Expect parameter name")
                )
                flag = False

        self._consume(TokenType.RIGHT_PAREN, "Expect ')' after parameters")

        self._consume(TokenType.LEFT_BRACE, "Expect '{' before" + f" {kind} body")

        body = self._block()
        return FunctionStmt(name, parameters, body)

    def _var_declaration(self):
        name: Token = self._consume(TokenType.IDENTIFIER, "Expect variable name")

        initializer: Expr = None

        if self._match(TokenType.EQUAL):
            initializer = self._expression()

        self._consume(TokenType.SEMICOLON, "Expect ';' after variable declaration")
        return VarStmt(name, initializer)

    def _statement(self):
        if self._match(TokenType.FOR):
            return self._for_statement()
        if self._match(TokenType.PRINT):
            return self._print_statement()
        if self._match(TokenType.RETURN):
            return self._return_statement()
        if self._match(TokenType.LEFT_BRACE):
            return BlockStmt(self._block())
        if self._match(TokenType.IF):
            return self._if_statement()
        return self._expression_statement()

    def _return_statement(self):
        keyword: Token = self._previous()
        value: Expr = None
        if not self._check(TokenType.SEMICOLON):
            value = self._expression()
        self._consume(TokenType.SEMICOLON, "Expected semicolon after return statement")
        return ReturnStmt(keyword, value)

    def _if_statement(self):
        self._consume(TokenType.LEFT_PAREN, "Expect '(' after 'if'.")

        condition: Expr = self._expression()

        self._consume(TokenType.RIGHT_PAREN, "Expect ')' after if condition")

        then_branch: Stmt = self._statement()

        else_branch: Stmt = None

        if self._match(TokenType.ELSE):
            else_branch = self._statement()

        return IfStmt(condition, then_branch, else_branch)

    def _for_statement(self) -> Stmt:
        self._consume(TokenType.LEFT_PAREN, "Expect '(' after 'for'.")

        initializer = None

        if self._match(TokenType.SEMICOLON):
            initializer = None
        elif self._match(TokenType.VAR):
            initializer = self._var_declaration()
        else:
            initializer = self._expression_statement()

        condition: Expr = None
        if not self._check(TokenType.SEMICOLON):
            condition = self._expression()

        self._consume(TokenType.SEMICOLON, "Expect ';' after loop condition")

        increment: Expr = None
        if not self._check(TokenType.RIGHT_PAREN):
            increment = self._expression()
        self._consume(TokenType.RIGHT_PAREN, "Expect ')' after for clauses")

        body: Stmt = self._statement()
        if increment:
            body = BlockStmt([body, ExpressionStmt(increment)])

        if condition is None:
            condition = LiteralExpr(True)
        body = WhileStmt(condition, body)

        if initializer:
            body = BlockStmt([initializer, body])
        return body

    def _print_statement(self) -> Stmt:
        value: Expr = self._expression()
        self._consume(TokenType.SEMICOLON, "Expect ';' after value.")

        return PrintStmt(value)

    def _expression_statement(self):
        expr: Expr = self._expression()
        self._consume(TokenType.SEMICOLON, "Expect ';' after expression")

        return ExpressionStmt(expr)

    def _while_statement(self):
        self._consume(TokenType.LEFT_PAREN, "Expect '(' after 'while'.")
        condition: Expr = self._expression()

        self._consume(TokenType.RIGHT_PAREN, "Expect ')' after condition")

        body: Stmt = self._statement()

        return WhileStmt(condition, body)

    def _expression(self) -> Expr:
        return self._assignment()

    def _block(self):
        stmts: list[Stmt] = []

        while not self._check(TokenType.RIGHT_BRACE) and not self._is_at_end():
            stmts.append(self._declaration())

        self._consume(TokenType.RIGHT_BRACE, "Expect '}' after block.")

        return stmts

    def _or(self):
        expr: Expr = self._and()

        while self._match(TokenType.OR):
            operator: Token = self._previous()

            right: Expr = self._and()

            expr = LogicalExpr(expr, operator, right)

        return expr

    def _and(self):
        expr: Expr = self._equality()

        while self._match(TokenType.AND):
            operator: Token = self._previous()

            right: Expr = self._equality()

            expr = LogicalExpr(expr, operator, right)

        return expr

    def _assignment(self) -> Expr:
        expr: Expr = self._or()
        if self._match(TokenType.EQUAL):
            equals: Token = self._previous()
            value: Expr = self._assignment()

            if isinstance(expr, VariableExpr):
                name: Token = expr.name
                return AssignExpr(name, value)
            elif isinstance(expr, GetExpr):
                get: GetExpr = expr
                return SetExpr(get.object, get.name, value)

            self.error(equals, "Invalid assignment target.")

        return expr

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
            # self.logger.info("Minus term", left=expr, right=right)
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

        return self._call()

    def _call(self) -> Expr:
        expr: Expr = self._primary()

        while True:
            if self._match(TokenType.LEFT_PAREN):
                expr = self._finish_call(expr)
            elif self._match(TokenType.DOT):
                name = self._consume(
                    TokenType.IDENTIFIER, "Expect property name after '.'."
                )
                expr = GetExpr(expr, name)
            else:
                break

        return expr

    def _finish_call(self, callee: Expr):
        arguments: list[Expr] = []

        if not self._check(TokenType.RIGHT_PAREN):
            # Can't do the lords loop aka do while
            flag = True
            while flag or self._match(TokenType.COMMA):
                flag = False
                arguments.append(self._expression())

                if len(arguments) >= 255:
                    self.error(self._peek(), "Why u gotta have that many arguments for")
        paren: Token = self._consume(
            TokenType.RIGHT_PAREN, "Expect ')' after arguments"
        )
        return CallExpr(callee, paren, arguments)

    def _primary(self) -> Expr:
        if self._match(TokenType.FALSE):
            return LiteralExpr(False)

        if self._match(TokenType.TRUE):
            return LiteralExpr(True)

        if self._match(TokenType.NIL):
            return LiteralExpr(None)

        if self._match(TokenType.NUMBER, TokenType.STRING):
            return LiteralExpr(self._previous().literal)

        if self._match(TokenType.SUPER):
            keyword: Token = self._previous()
            self._consume(TokenType.DOT, "Expect '.' after 'super'")
            method: Token = self._consume(
                TokenType.IDENTIFIER, "Expect superclass method name"
            )
            return SuperExpr(keyword, method)

        if self._match(TokenType.THIS):
            return ThisExpr(self._previous())

        if self._match(TokenType.IDENTIFIER):
            return VariableExpr(self._previous())

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
                    return self._for_statement()
                case TokenType.IF:
                    return self._if_statement()
                case TokenType.WHILE:
                    return self._while_statement()
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
