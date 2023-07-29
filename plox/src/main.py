import sys
from typing import Optional
from _token import Token
from log import get_logger

from scanner import Scanner


class Lox:
    def __init__(self, file_name: Optional[str] = None, logger = None) -> None:
        self.file_name: Optional[str] = file_name
        self.input = None
        self._logger = logger

    def run_file(self):
        if not self.file_name:
            raise Exception("Trying to read file without one assigned?")
        fp = open(self.file_name).readlines()
        self.input = "\n".join(fp)[:-1]
        self._run()

    def _run(self):
        if not self.input:
            raise Exception("Trying to run smth without input")

        self.scanner = Scanner(self.input, self._logger)
        self.tokens:list[Token] = self.scanner.scan_tokens()

        for token in self.tokens:
            print(token.to_string())

    def run_repl(self):
        while True:
            print(">", end="")
            inp = input().strip()

            if not inp:
                break

    @staticmethod
    def error(line: int, msg: str):
        Lox.report(line, "", msg)

    @staticmethod
    def report(line: int, where: str, message: str):
        raise Exception(f"[line {line}] Error {where} : {message}")


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
