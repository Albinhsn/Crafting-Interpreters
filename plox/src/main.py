import sys
from parser import Parser
from typing import Optional, Union

from _token import Token
from ast_printer import AstPrinter
from expression import Expr
from interpreter import Interpreter, LoxRuntimeError
from log import get_logger
from resolver import Resolver
from scanner import Scanner
from stmt import Stmt
from token_type import TokenType

HAD_ERROR = False


class Lox:
    def __init__(self, file_name: Optional[str] = None) -> None:
        self.file_name: Optional[str] = file_name
        self.input = None
        # self._logger = get_logger()

    def run_file(self):
        if not self.file_name:
            raise Exception("Trying to read file without one assigned?")
        fp = open(self.file_name).readlines()
        self.input = "\n".join(fp)[:-1]
        self._run()

    def _run(self):
        if not self.input:
            raise Exception("Trying to run smth without input")

        self.scanner = Scanner(self.input)
        self.tokens: list[Token] = self.scanner.scan_tokens()

        self.parser = Parser(self.tokens, self.error)
        stmts: list[Stmt] = self.parser.parse()

        self.interpreter = Interpreter(Lox.runtime_error)

        if HAD_ERROR:
            return

        self.resolver: Resolver = Resolver(self.interpreter)
        self.resolver.resolve(stmts)

        if HAD_ERROR:
            print("Had error")
            return

        self.interpreter.interpret(stmts)

    def run_repl(self):
        while True:
            print(">", end="")
            inp = input().strip()

            if not inp:
                break

    @staticmethod
    def error(line: Union[int, Token], msg: str):
        if isinstance(line, int):
            Lox.report(line, "", msg)
            return
        if line.type == TokenType.EOF:
            Lox.report(line.line, " at end", msg)
        else:
            Lox.report(line.line, f"at '{line.lexeme}'", msg)
        global HAD_ERROR
        HAD_ERROR = True

    @staticmethod
    def runtime_error(error: LoxRuntimeError):
        pass

    @staticmethod
    def report(line: int, where: str, message: str):
        print(f"[line {line}] Error {where} : {message}")


if __name__ == "__main__":
    args = sys.argv
    if len(args) > 2:
        print("Usage: plox [script]")
        exit(64)
    if len(args) == 2:
        lox = Lox(args[1])
        lox.run_file()
    else:
        lox = Lox()
        lox.run_repl()
