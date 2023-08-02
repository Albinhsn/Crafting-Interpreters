import sys

sys.path.append("./src")
from parser import Parser

from _token import Token
from interpreter import Interpreter
from scanner import Scanner
from stmt import BlockStmt, ExpressionStmt, FunctionStmt, Stmt, VarStmt
from token_type import TokenType


def pass_error(*args, **kwargs):
    pass


def run_code(txt: str):
    scanner: Scanner = Scanner(txt)
    scanner.scan_tokens()
    tokens: list[Token] = scanner.tokens
    parser: Parser = Parser(tokens, None)
    parser.error = pass_error
    interpreter: Interpreter = Interpreter(error=pass_error)
    interpreter.interpret(parser.parse())


def test_quick_maths(capsys):
    run_code("var a = 2 - 1;print a;")
    captured = capsys.readouterr()
    assert captured.out == "1\n"


def test_for(capsys):
    run_code(
        """for (var i = 0; i < 5; i = i + 1) {
        print i;
        }"""
    )
    captured = capsys.readouterr()
    assert captured.out == "0\n1\n2\n3\n4\n"


def test_func_return(capsys):
    run_code("fun fib(n) {return n;}var a = 5; print fib(a + 2);")
    captured = capsys.readouterr()
    assert captured.out == "7\n"


def test_func_recursion(capsys):
    run_code(
        "fun fib(n) {if (n < 2){return n;}return fib(n - 2) + fib(n - 1);} print fib(5);"
    )
    captured = capsys.readouterr()
    assert captured.out == "5\n"


# class Doughnut {
#   cook() {
#     print "Fry until golden brown.";
#   }
# }

# class BostonCream < Doughnut {
#   cook() {
#     super.cook();
#     print "Pipe full of custard and coat with chocolate.";
#   }
# }

# BostonCream().cook();

# fun fib(n) {
#   if (n <= 1){
#     return n;
#   }
#   return fib(n-2)  + fib(n-1);
# }

# for (var i = 0; i < 30; i = i + 1) {
#   print fib(i);
# }

# class DevonshireCream {
#   serveOn() {
#     return "Scones";
#   }
# }

# print DevonshireCream; // Prints "DevonshireCream".
