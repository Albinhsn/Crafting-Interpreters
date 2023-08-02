import sys

sys.path.append("./src")
from parser import Parser

from _token import Token
from interpreter import Interpreter
from log import get_logger
from main import Lox
from resolver import Resolver
from scanner import Scanner
from stmt import BlockStmt, ExpressionStmt, FunctionStmt, Stmt, VarStmt

LOGGER = get_logger()


def resolve(txt: str):
    scanner: Scanner = Scanner(txt)
    scanner.scan_tokens()
    tokens: list[Token] = scanner.tokens
    parser: Parser = Parser(tokens, None)
    parser.error = Lox.error
    interpreter: Interpreter = Interpreter(Lox.error)
    resolver: Resolver = Resolver(interpreter)
    return interpreter
    LOGGER.info(
        "resolved", locals=interpreter.locals, globals=interpreter.globals.values
    )


def test_resolve():
    interpreter = resolve(
        """
fun fib(n) {
    if (n < 2){return n;}
    var a = 5;
    return fib(n - 2) + fib(n - 1);
}
print fib(5);
            """
    )
