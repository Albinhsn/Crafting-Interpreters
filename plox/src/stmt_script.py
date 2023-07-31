import sys

types = []
if __name__ == "__main__":
    args = sys.argv
    if len(args) <= 1 or len(args) >= 3:
        raise Exception("Usage: expr_script <output_dir>")

    output_dir = args[1]
    fp = open(output_dir, "w")
    x = [
        "Expression ;  expression: Expr",
        "Print ;  expression: Expr",
        "Var ; name: Token, initializer: Expr",
        "Block ; stmts: list[Stmt]",
        "Var ; name:Token, initializer:Expr",
        "While ; condition:Expr, body:Stmt",
        "Block ; statements: list[Stmt]",
        "If ; condition : Expr, then_branch : Stmt, else_branch: Stmt",
        "Function ; name: Token, params: list[Token], body:list[Stmt]",
    ]
    k = """from abc import ABC
from typing import Any
from _token import Token
from expression import Expr, Visitor

class Stmt(ABC):
    def accept(self, a: Any):
        pass
"""

    for t in x:
        t = t.split(";")
        class_name = t[0].strip()
        fields = t[1].split(",")
        s = ""
        for f in fields:
            s += f"        self.{f} = {f.split(':')[0]}\n"
        k += f"""\nclass {class_name}Stmt(Stmt, Visitor):
    def __init__(self, {', '.join(fields)}):
{s}

    def accept(self, visitor: Visitor):
        visitor.visit_{class_name.lower()}_stmt(self)
        return visitor.visit_{class_name.lower()}_stmt(self)
        """
    fp.write(k)
