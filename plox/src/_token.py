from typing import Optional
from token_type import TokenType


class Token:
    def __init__(self, type: TokenType, lexeme: str, literal: Optional[str], line: int) -> None:
        self.type: TokenType= type
        self.lexeme = lexeme
        self.literal = literal
        self.line = line

    def to_string(self) -> str:
        return f"{self.type.name} {self.lexeme} {self.literal}"
