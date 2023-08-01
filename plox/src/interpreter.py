from typing import Any, Dict, Optional

from _token import Token
from environment import Environment
from expression import (AssignExpr, BinaryExpr, CallExpr, Expr, GetExpr,
                        GroupingExpr, LiteralExpr, LogicalExpr, SetExpr,
                        SuperExpr, ThisExpr, UnaryExpr, VariableExpr, Visitor)
from log import get_logger
from loxcallable import (Clock, LoxCallable, LoxClass, LoxFunction,
                         LoxInstance, Return)
from stmt import (BlockStmt, ClassStmt, ExpressionStmt, FunctionStmt, IfStmt,
                  PrintStmt, ReturnStmt, Stmt, VarStmt, WhileStmt)
from token_type import TokenType


class LoxRuntimeError(Exception):
    pass


class Interpreter(Visitor):
    def __init__(self, error) -> None:
        self.error = error
        self.environment = Environment()
        self.globals = Environment()
        self.globals._define("clock", Clock())
        self.logger = get_logger()
        self.__locals = {}

    def interpret(self, stmts: list[Stmt]):
        try:
            for stmt in stmts:
                # self.logger.info(
                #     "Executing stmt",
                #     env=self.environment.values,
                #     globals=self.globals.values,
                # )
                self._execute(stmt)
        except LoxRuntimeError as e:
            self.error(1, e.args[0])

    def _execute(self, stmt: Stmt):
        stmt.accept(self)

    def _resolve(self, expr: Expr, depth: int) -> None:
        self.__locals[expr] = depth

    def _stringify(self, obj: Any):
        if obj is None:
            return "nil"

        if isinstance(obj, float):
            txt: str = str(obj)

            if ".0" == txt[-2:]:
                txt = txt[0:-2]

            return txt
        if isinstance(obj, LoxClass):
            return obj.to_string()

        # Does this never raise?
        return str(obj)

    def visit_expression_stmt(self, stmt: ExpressionStmt):
        self._evaluate(stmt.expression)

    def visit_block_stmt(self, stmt: BlockStmt):
        self._execute_block(stmt.statements, self.environment)
        return

    def visit_class_stmt(self, stmt: ClassStmt):
        superclass: Any = None
        if stmt.superclass:
            superclass = self._evaluate(stmt.superclass)
            if not isinstance(superclass, LoxClass):
                raise Exception(f"{stmt.superclass.name} superclass must be a class")
        self.environment._define(stmt.name.lexeme, None)

        if stmt.superclass is not None:
            self.environment = Environment(self.environment)
            self.environment._define("super", superclass)

        methods: Dict[str, LoxFunction] = {}
        for method in stmt.methods:
            function: LoxFunction = LoxFunction(
                method, self.environment, method.name.lexeme == "init"
            )
            methods[method.name.lexeme] = function

        klass: LoxClass = LoxClass(stmt.name.lexeme, superclass, methods)

        if superclass is not None:
            if not self.environment.enclosing:
                raise Exception("Should be enclosing here")

            self.environment = self.environment.enclosing

        self.environment._assign(stmt.name, klass)

    def visit_function_stmt(self, stmt: FunctionStmt):
        function = LoxFunction(stmt, self.environment, False)
        self.environment._define(stmt.name.lexeme, function)
        return None

    def visit_if_stmt(self, stmt: IfStmt):
        if self._is_truthy(self._evaluate(stmt.condition)):
            self._execute(stmt.then_branch)
        elif stmt.else_branch is not None:
            self._execute((stmt.else_branch))

        return None

    def visit_print_stmt(self, stmt: PrintStmt):
        # self.logger.info(
        #     "Evaluating", exp=stmt.expression.name.lexeme, env=self.environment.values
        # )
        value: Any = self._evaluate(stmt.expression)
        print(self._stringify(value))

    def visit_var_stmt(self, stmt: VarStmt):
        value: Any = None
        if stmt.initializer is not None:
            value = self._evaluate(stmt.initializer)
        self.environment._define(stmt.name.lexeme, value)
        return None

    def visit_return_stmt(self, stmt: ReturnStmt):
        value = None
        if stmt.value is not None:
            # self.logger.info("Evaluating return stmt", value=stmt.value)
            value = self._evaluate(stmt.value)
            # self.logger.info("Evaluated return", value=value)
        raise Return(value)

    def visit_assign_expr(self, expr: AssignExpr):
        value: Any = self._evaluate(expr.value)

        distance: Optional[int] = self.__locals.get(expr)
        if distance:
            self.environment.__assign_at(distance, expr.name, value)
        else:
            self.globals._assign(expr.name, value)
        return value

    def visit_literal_expr(self, expr: LiteralExpr) -> Any:
        return expr.value

    def visit_variable_expr(self, expr: VariableExpr):
        return self.__lookup_variable(expr.name, expr)

    def __lookup_variable(self, name: Token, expr: Expr):
        distance: Optional[int] = self.__locals.get(expr)
        # self.logger.info(
        #     "Looking up variable",
        #     distance=distance,
        #     name=name.lexeme,
        #     env=self.environment.values,
        # )
        if distance:
            return self.environment.get_at(distance, name.lexeme)
        # This should be globals
        return self.environment.get(name)

    def visit_logical_expr(self, expr: LogicalExpr):
        left = self._evaluate(expr.left)

        if expr.operator.type == TokenType.OR:
            if self._is_truthy(left):
                return left
        else:
            if not self._is_truthy(left):
                return left
        return self._evaluate(expr.right)

    def visit_set_expr(self, expr: SetExpr):
        obj: Any = self._evaluate(expr.object)

        if not isinstance(object, LoxInstance):
            raise LoxRuntimeError(f"{expr.name} Only instances have fields")

        value: Any = self._evaluate(expr.value)
        obj.set(expr, value)
        return value

    def visit_super_expr(self, expr: SuperExpr):
        distance: Optional[int] = self.__locals.get(expr)
        if not distance:
            raise Exception("Should have distance here")
        superclass: LoxClass = self.environment.get_at(distance, "super")

        obj: LoxInstance = self.environment.get_at(distance - 1, "this")

        method: Optional[LoxFunction] = superclass.find_method(expr.method.lexeme)
        if not method:
            raise Exception(f"{expr.method.lexeme} Undefined property")

        return method.bind(obj)

    def visit_this_expr(self, expr: ThisExpr):
        return self.__lookup_variable(expr.keyword, expr)

    def visit_while_stmt(self, stmt: WhileStmt):
        while self._is_truthy(self._evaluate(stmt.condition)):
            self._execute(stmt.body)

    def visit_call_expr(self, expr: CallExpr):
        callee: Any = self._evaluate(expr.callee)
        arguments: list[Any] = [self._evaluate(argument) for argument in expr.arguments]
        # self.logger.info("Visiting call expr", args=arguments, env=self.environment.values)
        if not isinstance(callee, LoxCallable):
            self.error(expr.paren, "Can only call functions and classes")
            raise LoxRuntimeError("Trying to call non function")

        function: LoxCallable = callee

        if len(arguments) != function._arity():
            raise LoxRuntimeError(
                f"Expected {function._arity()} arguments but got {len(arguments)}."
            )
        returned = function._call(self, arguments)
        # self.logger.info("Returned from func", returned=returned, current_env=self.environment.values)
        return returned

    def visit_get_expr(self, expr: GetExpr) -> Any:
        obj: Any = self._evaluate(expr.object)
        if isinstance(obj, LoxInstance):
            return obj.get(expr.name)

        self.error(expr.name, "Only instances have properties")

    def visit_unary_expr(self, expr: UnaryExpr):
        right: Any = self._evaluate(expr.right)

        match expr.operator.type:
            case TokenType.BANG:
                return not self._is_truthy(right)
            case TokenType.MINUS:
                self._check_number_operand(expr.operator, right)
                return -float(right)
            case _:
                pass

        return None

    def _is_truthy(self, obj: Any):
        if not obj:
            return False

        if isinstance(obj, bool):
            return bool(obj)

        return True

    def visit_binary_expr(self, expr: BinaryExpr):
        left: Any = self._evaluate(expr.left)
        right: Any = self._evaluate(expr.right)

        # self.logger.info("Binary expr", left=left, right=right)
        match expr.operator.type:
            case TokenType.BANG_EQUAL:
                return left != right

            case TokenType.EQUAL:
                return left == right

            case TokenType.GREATER:
                self._check_number_operands(expr.operator, left, right)
                return left > right

            case TokenType.GREATER_EQUAL:
                self._check_number_operands(expr.operator, left, right)
                return left >= right

            case TokenType.LESS:
                self._check_number_operands(expr.operator, left, right)
                return left < right

            case TokenType.LESS_EQUAL:
                self._check_number_operands(expr.operator, left, right)
                return left <= right

            case TokenType.MINUS:
                return float(left) - float(right)

            case TokenType.PLUS:
                if isinstance(left, float) and isinstance(right, float):
                    return left + right

                if isinstance(left, str) and isinstance(right, str):
                    return left + right
                raise Exception(f"Trying to + smth? {type(left)} and {type(right)}")

            case TokenType.SLASH:
                self._check_number_operands(expr.operator, left, right)
                return float(left) / float(right)

            case TokenType.STAR:
                self._check_number_operands(expr.operator, left, right)
                return float(left) * float(right)

        return None

    def visit_grouping_expr(self, expr: GroupingExpr):
        return self._evaluate(expr)

    def _check_number_operand(self, operator: Token, operand: Any):
        if isinstance(operand, float):
            return

        raise LoxRuntimeError(operator, "Operand must be a number")

    def _execute_block(self, statements: list[Stmt], environment: Environment):
        previous: Environment = self.environment
        self.environment = environment

        try:
            for statement in statements:
                # self.logger.info("Executing block stmt", stmt=statement)
                self._execute(statement)
        except Return as r:
            self.environment = previous
            raise r

        self.environment = previous

    def _check_number_operands(self, operator: Token, left_operand: Any, right_operand):
        if isinstance(left_operand, float) and isinstance(right_operand, float):
            return

        raise LoxRuntimeError(operator, "Operands must be a numbers")

    def _evaluate(self, expr: Expr):
        return expr.accept(self)
