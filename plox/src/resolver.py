from typing import Union

from _token import Token
from expression import Expr, Visitor
from stack import Stack
from stmt import BlockStmt, Stmt, VarStmt


class Resolver(Visitor):
    def __init__(self, interpreter) -> None:
        self.interpreter = interpreter
        self.scopes = Stack()

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

    def __end_scope(self):
        self.scopes.pop()

    def visit_block_stmt(self, stmt: BlockStmt) -> None:
        self.__begin_scope()
        self.__resolve(stmt.statements)
        self.__end_scope()

    def visit_var_stmt(self, stmt: VarStmt) -> None:
        self.__declare(stmt.name)
        if stmt.initializer != None:
            self.__resolve(stmt.initializer)
        self.__define(stmt.name)

    def __declare(self, name: Token):
        if self.scopes.length == 0:
            return
        scope: dict = self.scopes.peek()
        scope[name.lexeme] = False

    def __define(self, name: Token) -> None:
        if self.scopes.length == 0:
            return

        self.scopes.peek()[name] = True
