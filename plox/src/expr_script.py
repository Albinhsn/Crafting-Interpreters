import sys

types = []
if __name__ == "__main__":
    args = sys.argv
    if len(args) <= 1 or len(args) >= 3:
        raise Exception("Usage: expr_script <output_dir>")

    output_dir = args[1]
    fp = open(output_dir, "w")
    fp.write(
        """from abc import ABC
from _token import Token

class Expr(ABC):
    pass
    """
    )
    x = [
            "Binary   ;left: Expr, operator: Token, right: Expr",
            "Grouping ;expr: Expr",
            "Literal  ;value: dict",
            "Unary    ;operator: Token,right:Expr",
    ]
    for t in x:
        t = t.split(";")
        class_name = t[0].strip()
        fields = t[1].split(",")
        s = ""
        for f in fields:
            s += f"        self.{f} = {f.split(':')[0]}\n"
        k = f"""\nclass {class_name}(Expr):
    def __init__(self, {', '.join(fields)}):
{s}
        """
        fp.write(k)
