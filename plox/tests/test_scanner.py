import sys

sys.path.append("./src")
from _token import Token
from log import get_logger
from scanner import Scanner
from token_type import TokenType

LOGGER = get_logger()


def get_tokens(s: str):
    scanner = Scanner(s, LOGGER)
    scanner.scan_tokens()
    return scanner


def test_random_code():
    scanner = get_tokens("var fem = 5\nvar sju = fem + 2")
    assert scanner.tokens[0].type == TokenType.VAR
    
    assert scanner.tokens[1].type == TokenType.IDENTIFIER
    assert scanner.tokens[1].literal == "fem" 

    assert scanner.tokens[2].type == TokenType.EQUAL 

    assert scanner.tokens[3].type == TokenType.NUMBER
    assert scanner.tokens[3].literal == 5

    assert scanner.tokens[4].type == TokenType.VAR

    assert scanner.tokens[5].type == TokenType.IDENTIFIER
    assert scanner.tokens[5].literal == "sju" 

    assert scanner.tokens[6].type == TokenType.EQUAL 

    assert scanner.tokens[7].type == TokenType.IDENTIFIER
    assert scanner.tokens[7].literal == "fem" 
    
    assert scanner.tokens[8].type == TokenType.PLUS

    assert scanner.tokens[9].type == TokenType.NUMBER
    assert scanner.tokens[9].literal == 2 

def test_identifiers():
    scanner = get_tokens("testing this stuff")
    assert scanner.tokens[0].type == TokenType.IDENTIFIER
    assert scanner.tokens[0].literal == "testing"

    assert scanner.tokens[1].type == TokenType.THIS
    assert scanner.tokens[1].literal == "this"

    assert scanner.tokens[2].type == TokenType.IDENTIFIER
    assert scanner.tokens[2].literal == "stuff"

def test_single_chars():
    scanner = get_tokens("(){},.-+;=*!<>")
    
    assert scanner.tokens[0].type == TokenType.LEFT_PAREN
    assert scanner.tokens[1].type == TokenType.RIGHT_PAREN
    assert scanner.tokens[2].type == TokenType.LEFT_BRACE
    assert scanner.tokens[3].type == TokenType.RIGHT_BRACE
    assert scanner.tokens[4].type == TokenType.COMMA
    assert scanner.tokens[5].type == TokenType.DOT
    assert scanner.tokens[6].type == TokenType.MINUS
    assert scanner.tokens[7].type == TokenType.PLUS
    assert scanner.tokens[8].type == TokenType.SEMICOLON
    assert scanner.tokens[9].type == TokenType.EQUAL
    assert scanner.tokens[10].type == TokenType.STAR
    assert scanner.tokens[11].type == TokenType.BANG
    assert scanner.tokens[12].type == TokenType.LESS
    assert scanner.tokens[13].type == TokenType.GREATER

    assert scanner.tokens[14].type == TokenType.EOF

def test_double_chars():
    scanner = get_tokens("!===<=>=")

    assert scanner.tokens[0].type == TokenType.BANG_EQUAL
    assert scanner.tokens[1].type == TokenType.EQUAL_EQUAL
    assert scanner.tokens[2].type == TokenType.LESS_EQUAL
    assert scanner.tokens[3].type == TokenType.GREATER_EQUAL

def test_numbers():
    scanner = get_tokens("1 1.0 11.01")

    assert scanner.tokens[0].type == TokenType.NUMBER
    assert scanner.tokens[0].literal == 1.0

    assert scanner.tokens[1].type == TokenType.NUMBER
    assert scanner.tokens[1].literal == 1.0

    assert scanner.tokens[2].type == TokenType.NUMBER
    assert scanner.tokens[2].literal == 11.01

    assert scanner.tokens[3].type == TokenType.EOF

def test_keywords():
    scanner = get_tokens(
        "and class else false for fun if nil or print return super this true var while"
    )

    assert scanner.tokens[0].type == TokenType.AND
    assert scanner.tokens[0].literal == "and" 

    assert scanner.tokens[1].type == TokenType.CLASS
    assert scanner.tokens[1].literal == "class" 

    assert scanner.tokens[2].type == TokenType.ELSE
    assert scanner.tokens[2].literal == "else"

    assert scanner.tokens[3].type == TokenType.FALSE
    assert scanner.tokens[3].literal == "false"

    assert scanner.tokens[4].type == TokenType.FOR
    assert scanner.tokens[4].literal == "for"

    assert scanner.tokens[5].type == TokenType.FUN
    assert scanner.tokens[5].literal == "fun"

    assert scanner.tokens[6].type == TokenType.IF
    assert scanner.tokens[6].literal == "if"

    assert scanner.tokens[7].type == TokenType.NIL
    assert scanner.tokens[7].literal == "nil"

    assert scanner.tokens[8].type == TokenType.OR
    assert scanner.tokens[8].literal == "or"

    assert scanner.tokens[9].type == TokenType.PRINT
    assert scanner.tokens[9].literal == "print"

    assert scanner.tokens[10].type == TokenType.RETURN
    assert scanner.tokens[10].literal == "return"

    assert scanner.tokens[11].type == TokenType.SUPER
    assert scanner.tokens[11].literal == "super"

    assert scanner.tokens[12].type == TokenType.THIS
    assert scanner.tokens[12].literal == "this"

    assert scanner.tokens[13].type == TokenType.TRUE
    assert scanner.tokens[13].literal == "true"

    assert scanner.tokens[14].type == TokenType.VAR
    assert scanner.tokens[14].literal == "var"

    assert scanner.tokens[15].type == TokenType.WHILE
    assert scanner.tokens[15].literal == "while"

    assert scanner.tokens[16].type == TokenType.EOF
