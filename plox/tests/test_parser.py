import sys


sys.path.append("./src")
from parser import Parser

from token_type import TokenType

from _token import Token
from scanner import Scanner
from stmt import BlockStmt, FunctionStmt, Stmt, VarStmt, ExpressionStmt


def pass_error(*args, **kwargs):
    pass


def get_stmts(txt) -> list[Stmt]:
    scanner: Scanner = Scanner(txt)
    scanner.scan_tokens()
    tokens: list[Token] = scanner.tokens
    parser: Parser = Parser(tokens, None)
    parser.error = pass_error
    return parser.parse()

def test_quick_maths():
    stmts: list[Stmt] = get_stmts("var a = 2 - 1;")

def test_rec_fib():
    stmts: list[Stmt] = get_stmts(
        """
fun fib(n) {
  if (n < 2) return n;
  return fib(n - 2) + fib(n - 1);
}

for (var i = 0; i < 20; i = i + 1) {
  print fib(i);
}
"""
    )
    assert type(stmts[0]) == FunctionStmt
    assert type(stmts[1]) == BlockStmt
    assert len(stmts[0].body) == 2  
    assert stmts[0].name.literal == "fib"
    assert stmts[0].params[0].literal == "n" 


def test_assignment():
    stmts: list[Stmt] = get_stmts("var a = 5;")
    assert len(stmts) == 1
    assert type(stmts[0]) == VarStmt
    assert stmts[0].name.literal == "a"
    assert stmts[0].name.lexeme == "a"
    assert stmts[0].initializer.value == 5
