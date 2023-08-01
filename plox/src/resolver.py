from typing import Union

from expression import Expr, Visitor
from stmt import BlockStmt, Stmt


class Resolver(Visitor):
    def __init__(self, interpreter) -> None:
        self.interpreter = interpreter

    def visit_block_stmt(self, stmt: BlockStmt) -> None:
        self.__begin_scope()
        self.__resolve(stmt.statements)
        self.__end_scope()

    def __resolve(self, res: Union[list[Stmt], Stmt, Expr]) -> None:
        if isinstance(res, list):
            for stmt in res:
                self.__resolve(stmt)

        elif isinstance(res, Stmt):
            res.accept(self)

        elif isinstance(res, Expr):
            res.accept(self)
        else:
            raise Exception(f"Can't resolve type {type(res)}")

    def __begin_scope(self):
        self.scopes.push({})
