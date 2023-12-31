from expression import (BinaryExpr, Expr, GroupingExpr, LiteralExpr, UnaryExpr,
                        Visitor)


class AstPrinter(Visitor):
    def __init__(self) -> None:
        pass

    def print(self, expr: Expr):
        return expr.accept(self)

    def visit_binary_epxr(self, expr: BinaryExpr):
        return self._parenthesize(
            name=expr.operator.lexeme, exprs=[expr.left, expr.right]
        )

    def visit_grouping_expr(self, expr: GroupingExpr):
        return self._parenthesize("group", [expr.expression])

    def visit_literal_expr(self, expr: LiteralExpr):
        return str(expr.value) if expr.value else "null"

    def visist_unary_expr(self, expr: UnaryExpr):
        return self._parenthesize(name=expr.operator.lexeme, exprs=[expr.right])

    def _parenthesize(self, name: str, exprs: list[Expr]):
        return f"( {name}" + " ".join([expr.accept(self) for expr in exprs]) + ")"
