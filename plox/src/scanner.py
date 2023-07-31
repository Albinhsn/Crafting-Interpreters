from typing import Any, Optional, Union

from structlog._config import BoundLoggerLazyProxy

from _token import Token
from log import get_logger
from token_type import TokenType


class Scanner:
    def __init__(self, source: str) -> None:
        self.source: str = source
        self.tokens = []
        self._start = 0
        self._current = -1
        self._line = 1
        self._logger: BoundLoggerLazyProxy= get_logger() 
        self.keywords = {
            "and": TokenType.AND,
            "class": TokenType.CLASS,
            "else": TokenType.ELSE,
            "false": TokenType.FALSE,
            "for": TokenType.FOR,
            "fun": TokenType.FUN,
            "if": TokenType.IF,
            "nil": TokenType.NIL,
            "or": TokenType.OR,
            "print": TokenType.PRINT,
            "return": TokenType.RETURN,
            "super": TokenType.SUPER,
            "this": TokenType.THIS,
            "true": TokenType.TRUE,
            "var": TokenType.VAR,
            "while": TokenType.WHILE,
        }

    def _is_at_end(self) -> bool:
        return self._current >= len(self.source)-1

    def scan_tokens(self) -> list[Token]:
        while not self._is_at_end():
            self._scan_token()

        self.tokens.append(Token(TokenType.EOF, "", None, self._line))

        return self.tokens

    def _keyword(self, word: str) -> Union[TokenType, None]:
        return self.keywords[word] if word in self.keywords else None

    def _scan_token(self) -> None:
        c: str = self._advance()
        match c:
            case "(":
                self._add_token(TokenType.LEFT_PAREN)
            case ")":
                self._add_token(TokenType.RIGHT_PAREN)
            case "{":
                self._add_token(TokenType.LEFT_BRACE)
            case "}":
                self._add_token(TokenType.RIGHT_BRACE)
            case ",":
                self._add_token(TokenType.COMMA)
            case ".":
                self._add_token(TokenType.DOT)
            case "-":
                self._add_token(TokenType.MINUS)
            case "+":
                self._add_token(TokenType.PLUS)
            case ";":
                self._add_token(TokenType.SEMICOLON)
            case "*":
                self._add_token(TokenType.STAR)
            case "!":
                self._add_token(
                    TokenType.BANG_EQUAL if self._match("=") else TokenType.BANG
                )
            case "=":
                self._add_token(
                    TokenType.EQUAL_EQUAL if self._match("=") else TokenType.EQUAL
                )
            case "<":
                self._add_token(
                    TokenType.LESS_EQUAL if self._match("=") else TokenType.LESS
                )
            case ">":
                self._add_token(
                    TokenType.GREATER_EQUAL if self._match("=") else TokenType.GREATER
                )
            case "/":
                if self._match("/"):
                    while self._peek() != "\n" and not self._is_at_end():
                        self._advance()
                else:
                    self._add_token(TokenType.SLASH)
            case " ":
                pass
            case "\r":
                pass
            case "\t":
                pass
            case "\n":
                self._line += 1
            case '"':
                self._string()

            case _:
                if self._is_digit(c):
                    self._number()
                elif self._is_alpha(c):
                    self._identifier()
                else:
                    raise Exception(f"Found unknown token {c}")

    def _is_alpha(self, c: str) -> bool:
        return (c >= "a" and c <= "z") or (c >= "A" and c <= "Z") or (c == "_")

    def _is_alpha_numeric(self, c: str) -> bool:
        return self._is_alpha(c) or self._is_digit(c)

    def _identifier(self):
        self._start = self._current
        while self._is_alpha_numeric(self._peek_next()):
            self._advance()

        txt: str = self.source[self._start : self._current+1]
        type: Union[TokenType, None] = self._keyword(txt)
        if not type:
            type = TokenType.IDENTIFIER

        self._add_token(type, txt)

    def _number(self) -> None:
        self._start = self._current

        # Hacky solution to solve when last thing is number
        if self._is_at_end():
            self._current += 1

        while self._is_digit(self._peek()):
            self._advance()
            
        if self._peek() == "." and self._is_digit(self._peek_next()):
            # Consume the "."
            self._advance()

            while self._is_digit(self._peek_next()):
                self._advance()


        self._add_token(
            TokenType.NUMBER, float(self.source[self._start : self._current])
        )
        self._current -= 1

    def _string(self) -> None:
        self._advance()
        self._start = self._current

        while self._peek_next() != '"' and not self._is_at_end():
            if self._peek() == "\n":
                self._line += 1
            self._advance()

        if self._is_at_end():
            raise Exception("Unterminated string")

        value: str = self.source[self._start: self._current+1]
        self._advance()
        self._add_token(TokenType.STRING, value)

    def _peek_next(self) -> str:
        if self._current + 1 >= len(self.source):
            return "\0"

        return self.source[self._current + 1]

    def _peek(self) -> str:
        return "\0" if self._is_at_end() else self.source[self._current]

    def _is_digit(self, c: str) -> bool:
        return "0" <= c and c <= "9"

    def _advance(self) -> str:
        self._current += 1
        return self.source[self._current]

    def _add_token(self, type: TokenType, literal: Optional[Any] = None) -> None:
        txt: str = self.source[self._start : self._current]
        self.tokens.append(Token(type, txt, literal, self._line))

    def _match(self, expected: str) -> bool:
        if self._is_at_end():
            return False
        if self.source[self._current+1] != expected:
            return False

        self._current += 1
        return True
