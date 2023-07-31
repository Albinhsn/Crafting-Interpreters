import sys


sys.path.append("./src")
from parser import Parser

from _token import Token
from scanner import Scanner
from stmt import Stmt, VarStmt



def get_stmts(txt) -> list[Stmt]:
    scanner: Scanner = Scanner(txt)
    scanner.scan_tokens()
    tokens: list[Token] = scanner.tokens
    parser: Parser = Parser(tokens, None)

    return parser.parse()


def test_assignment():
    stmts: list[Stmt] = get_stmts("var a = 5;")
    assert len(stmts) == 1
    assert type(stmts[0]) == VarStmt
    assert stmts[0].name.literal == "a"
    assert stmts[0].name.lexeme == "a"
    assert stmts[0].initializer.value == 5
