from token import Token
from typing import Optional

from token_type import TokenType


class Scanner:
    def __init__(self, source: str) -> None:
        self.source: str = source
        self.tokens = []
        self._start = 0
        self._current = 0
        self._line = 1

    def _is_at_end(self):
        return self._current >= len(self.source)-1

    def scan_tokens(self) -> list[Token]:
        while not self._is_at_end():
            self.start = self._current
            self._scan_token()

        self.tokens.append(Token(TokenType.EOF, "", None, self._line))

        return self.tokens

    def _scan_token(self):
        c: str = self._advance()
        single_map = {
            "(": TokenType.LEFT_PAREN,
            ")": TokenType.RIGHT_PAREN,
            "{": TokenType.LEFT_BRACE,
            "}": TokenType.RIGHT_BRACE,
            ",": TokenType.COMMA,
            ".": TokenType.DOT,
            "-": TokenType.MINUS,
            "+": TokenType.PLUS,
            ";": TokenType.SEMICOLON,
            "*": TokenType.STAR,
        }
        if c in single_map:
            self._add_token(single_map[c])
        if c == "!":
            self._add_token(
                TokenType.BANG_EQUAL if self._match("=") else TokenType.BANG
            )
        if c == "=":
            self._add_token(
                TokenType.EQUAL_EQUAL if self._match("=") else TokenType.EQUAL
            )
        if c == "<":
            self._add_token(
                TokenType.LESS_EQUAL if self._match("=") else TokenType.LESS
            )
        if c == ">":
            self._add_token(
                TokenType.GREATER_EQUAL if self._match("=") else TokenType.GREATER
            )

    def _advance(self):
        self._current += 1
        return self.source[self._current]

    def _add_token(self, type: TokenType, literal: Optional[dict] = None):
        txt: str = self.source[self._start : self._current]

        self.tokens.append(Token(type, txt, literal, self._line))

    def _match(self, expected: str) -> bool:
        if self._is_at_end():
            return False
        if self.source[self._current] != expected:
            return False

        self._current += 1
        return True
