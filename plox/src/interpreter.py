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
        self.globals = self.environment
        self.globals.define("clock", Clock())
        self.logger = get_logger()
        self.locals = {}

    def get_all_env(self):
        envs = []
        env = self.environment
        while env:
            envs.append(env.values)
            env = env.enclosing
        return envs

    def interpret(self, stmts: list[Stmt]):
        try:
            for stmt in stmts:
                self.execute(stmt)
        except LoxRuntimeError as e:
            self.error(1, e.args[0])

    def execute(self, stmt: Stmt):
        stmt.accept(self)

    def resolve(self, expr: Expr, depth: int) -> None:
        self.locals[expr] = depth

    def stringify(self, obj: Any):
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
        self.evaluate(stmt.expression)

    def visit_block_stmt(self, stmt: BlockStmt):
        self.execute_block(stmt.statements, Environment(self.environment))
        return

    def visit_class_stmt(self, stmt: ClassStmt):
        superclass: Any = None
        if stmt.superclass:
            superclass = self.evaluate(stmt.superclass)
            if not isinstance(superclass, LoxClass):
                raise Exception(f"{stmt.superclass.name} superclass must be a class")

        self.environment.define(stmt.name.lexeme, None)

        if stmt.superclass is not None:
            self.environment = Environment(self.environment)
            self.environment.define("super", superclass)

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

        self.environment.assign(stmt.name, klass)

    def visit_function_stmt(self, stmt: FunctionStmt):
        function = LoxFunction(stmt, self.environment, False)
        self.environment.define(stmt.name.lexeme, function)
        return None

    def visit_if_stmt(self, stmt: IfStmt):
        if self._is_truthy(self.evaluate(stmt.condition)):
            self.execute(stmt.then_branch)
        elif stmt.else_branch is not None:
            self.execute((stmt.else_branch))

        return None

    def visit_print_stmt(self, stmt: PrintStmt):
        value: Any = self.evaluate(stmt.expression)
        print(self.stringify(value))

    def visit_var_stmt(self, stmt: VarStmt):
        value: Any = None
        if stmt.initializer is not None:
            value = self.evaluate(stmt.initializer)
        self.environment.define(stmt.name.lexeme, value)
        return None

    def visit_return_stmt(self, stmt: ReturnStmt):
        value = None
        if stmt.value is not None:
            value = self.evaluate(stmt.value)
        raise Return(value)

    def visit_assign_expr(self, expr: AssignExpr):
        value: Any = self.evaluate(expr.value)

        distance: Optional[int] = self.locals.get(expr)
        if distance:
            self.environment.assign_at(distance, expr.name, value)
        else:
            self.globals.assign(expr.name, value)
        return value

    def visit_literal_expr(self, expr: LiteralExpr) -> Any:
        return expr.value

    def visit_variable_expr(self, expr: VariableExpr):
        return self.lookup_variable(expr.name, expr)

    def lookup_variable(self, name: Token, expr: Expr):
        distance: Optional[int] = self.locals.get(expr)
        if distance is not None:
            var = self.environment.get_at(distance, name.lexeme)
            return var
        # This should be globals
        return self.environment.get(name)

    def visit_logical_expr(self, expr: LogicalExpr):
        left = self.evaluate(expr.left)

        if expr.operator.type == TokenType.OR:
            if self._is_truthy(left):
                return left
        else:
            if not self._is_truthy(left):
                return left
        return self.evaluate(expr.right)

    def visit_set_expr(self, expr: SetExpr):
        obj: Any = self.evaluate(expr.object)

        if not isinstance(object, LoxInstance):
            raise LoxRuntimeError(f"{expr.name} Only instances have fields")

        value: Any = self.evaluate(expr.value)
        obj.set(expr, value)
        return value

    def visit_super_expr(self, expr: SuperExpr):
        distance: Optional[int] = self.locals.get(expr)
        if distance is None:
            raise Exception("Should have distance here")
        superclass: LoxClass = self.environment.get_at(distance, "super")

        obj: LoxInstance = self.environment.get_at(distance - 1, "this")
        method: Optional[LoxFunction] = superclass.find_method(expr.method.lexeme)
        if method is None:
            raise Exception(f"{expr.method.lexeme} Undefined property")

        return method.bind(obj)

    def visit_this_expr(self, expr: ThisExpr):
        return self.lookup_variable(expr.keyword, expr)

    def visit_while_stmt(self, stmt: WhileStmt):
        while self._is_truthy(self.evaluate(stmt.condition)):
            self.execute(stmt.body)

    def visit_call_expr(self, expr: CallExpr):
        callee: Any = self.evaluate(expr.callee)
        arguments: list[Any] = [self.evaluate(argument) for argument in expr.arguments]
        if not isinstance(callee, LoxCallable):
            self.error(expr.paren, "Can only call functions and classes")
            raise LoxRuntimeError("Trying to call non function")

        function: LoxCallable = callee

        if len(arguments) != function._arity():
            raise LoxRuntimeError(
                f"Expected {function._arity()} arguments but got {len(arguments)}."
            )
        returned = function._call(self, arguments)
        return returned

    def visit_get_expr(self, expr: GetExpr) -> Any:
        obj: Any = self.evaluate(expr.object)
        if isinstance(obj, LoxInstance):
            return obj.get(expr.name)

        self.error(expr.name, "Only instances have properties")

    def visit_unary_expr(self, expr: UnaryExpr):
        right: Any = self.evaluate(expr.right)

        match expr.operator.type:
            case TokenType.BANG:
                return not self._is_truthy(right)
            case TokenType.MINUS:
                self.check_number_operand(expr.operator, right)
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
        left: Any = self.evaluate(expr.left)
        right: Any = self.evaluate(expr.right)
        # self.logger.info("Visited binary expr", left=left, right=right)

        match expr.operator.type:
            case TokenType.BANG_EQUAL:
                return left != right

            case TokenType.EQUAL:
                return left == right

            case TokenType.GREATER:
                self.check_number_operands(expr.operator, left, right)
                return left > right

            case TokenType.GREATER_EQUAL:
                self.check_number_operands(expr.operator, left, right)
                return left >= right

            case TokenType.LESS:
                self.check_number_operands(expr.operator, left, right)
                return left < right

            case TokenType.LESS_EQUAL:
                self.check_number_operands(expr.operator, left, right)
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
                self.check_number_operands(expr.operator, left, right)
                return float(left) / float(right)

            case TokenType.STAR:
                self.check_number_operands(expr.operator, left, right)
                return float(left) * float(right)

        return None

    def visit_grouping_expr(self, expr: GroupingExpr):
        return self.evaluate(expr)

    def check_number_operand(self, operator: Token, operand: Any):
        if isinstance(operand, float):
            return

        raise LoxRuntimeError(operator, "Operand must be a number")

    def execute_block(self, statements: list[Stmt], environment: Environment):
        previous: Environment = self.environment
        self.environment = environment
        try:
            for statement in statements:
                self.execute(statement)
        except Return as r:
            self.environment = previous
            raise r
        self.environment = previous

    def check_number_operands(self, operator: Token, left_operand: Any, right_operand):
        if isinstance(left_operand, float) and isinstance(right_operand, float):
            return

        raise LoxRuntimeError(operator, "Operands must be a numbers")

    def evaluate(self, expr: Expr):
        return expr.accept(self)
