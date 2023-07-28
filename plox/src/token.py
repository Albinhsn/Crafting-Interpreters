from token_type import TokenType


class Token:
    def __init__(self, type: TokenType, lexeme: str, literal: str, line: int) -> None:
        self.type = type
        self.lexeme = lexeme
        self.literal = literal
        self.line = line

    def to_string(self) -> str:
        return f"{type} {self.lexeme} {self.literal}"
