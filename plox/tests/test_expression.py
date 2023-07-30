import sys

sys.path.append("./src")
from ast_printer import AstPrinter
from expression import BinaryExpr
from log import get_logger

LOGGER = get_logger()


def test_ast_printer():
    printer = AstPrinter()
