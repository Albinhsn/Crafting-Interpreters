import sys

types = []
if __name__ == "__main__":
    args = sys.argv
    if len(args) <= 1 or len(args) >= 3:
        raise Exception("Usage: expr_script <output_dir>")

    output_dir = args[1]
    fp = open(output_dir, "w")
    x = [
        "Assign ; name: Token, value: Expr",
        "Binary   ;left: Expr, operator: Token, right: Expr",
        "Grouping ;expression: Expr",
        "Logical ; left: Expr, operator: Token, right: Expr",
        "Literal  ;value: Any",
        "Unary    ;operator: Token,right:Expr",
        "Set ; object: Expr, name:Token, value:Expr",
        "This ; keyword:Token",
        "Variable ; name: Token",
        "Call ; callee : Expr, paren : Token, arguments : list[Expr]",
        "Get ; object: Expr, name: Token",
    ]
    k = """from abc import ABC
from typing import Any
from _token import Token
class Visitor(ABC):
    pass
"""
    s = ""
    for t in x:
        t = t.split(";")
        class_name = t[0].strip().lower()
        s += f"""
    def visit_{class_name}_expr(self, cls):
        pass
"""
    STMTS = [
        "var",
        "print",
        "expression",
        "block",
        "if",
        "while",
        "function",
        "return",
        "class",
    ]
    for i in STMTS:
        s += f"""
    def visit_{i}_stmt(self, cls):
        pass
"""
    k += s
    k += """class Expr(ABC):
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
        k += f"""\nclass {class_name}Expr(Expr, Visitor):
    def __init__(self, {', '.join(fields)}):
{s}

    def accept(self, visitor: Visitor):
        return visitor.visit_{class_name.lower()}_expr(self)
        """
    fp.write(k)
