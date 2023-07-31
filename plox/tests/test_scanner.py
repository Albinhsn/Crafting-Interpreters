import sys

sys.path.append("../src")
from _token import Token
from log import get_logger
from scanner import Scanner
from token_type import TokenType

LOGGER = get_logger()


def get_tokens(s: str):
    scanner = Scanner(s)
    scanner.scan_tokens()
    return scanner


def test_less():
    scanner = get_tokens("1 < 5")
    assert scanner.tokens[1].type == TokenType.LESS
    assert scanner.tokens[1].literal == "<" 


def test_func():
    scanner = get_tokens('sayHi("Dear", "Reader");')
    assert [i.type.name for i in scanner.tokens] == [
        "IDENTIFIER",
        "LEFT_PAREN",
        "STRING",
        "COMMA",
        "STRING",
        "RIGHT_PAREN",
        "SEMICOLON",
        "EOF",
    ]
    assert scanner.tokens[0].literal == "sayHi"
    assert scanner.tokens[0].lexeme == "sayHi"
    assert scanner.tokens[0].type == TokenType.IDENTIFIER

    assert scanner.tokens[1].literal == "("
    assert scanner.tokens[1].lexeme == "("
    assert scanner.tokens[1].type == TokenType.LEFT_PAREN

    assert scanner.tokens[2].literal == "Dear"
    assert scanner.tokens[2].lexeme == "Dear"
    assert scanner.tokens[2].type == TokenType.STRING

    assert scanner.tokens[3].type == TokenType.COMMA
    assert scanner.tokens[3].literal == ","
    assert scanner.tokens[3].lexeme == ","

    assert scanner.tokens[4].literal == "Reader"
    assert scanner.tokens[4].lexeme == "Reader"
    assert scanner.tokens[4].type == TokenType.STRING


def test_lexeme():
    scanner = get_tokens("a < 5")
    assert scanner.tokens[0].literal == "a"
    assert scanner.tokens[0].lexeme == "a"

    assert scanner.tokens[1].literal == "<"
    assert scanner.tokens[1].lexeme == "<"


def test_fibonacci():
    scanner = get_tokens(
        """
var a = 0;
var temp;

for (var b = 1; a < 5; b = temp + b) {
  print a;
  temp = a;
  a = b;
}

                         """
    )
    assert scanner.tokens[1].literal == "a"
    assert scanner.tokens[6].literal == "temp"
    assert [i.type.name for i in scanner.tokens] == [
        "VAR",  # var 1
        "IDENTIFIER",  # a 2
        "EQUAL",  # = 3
        "NUMBER",  # 0 4
        "SEMICOLON",  # ; 5
        "VAR",  # var 6
        "IDENTIFIER",  # temp 7
        "SEMICOLON",  # ; 8
        "FOR",  # for 9
        "LEFT_PAREN",  # ( 10
        "VAR",  # var 11
        "IDENTIFIER",  # b 12
        "EQUAL",  # = 13
        "NUMBER",  # 1 14
        "SEMICOLON",  # ; 15
        "IDENTIFIER",  # a 16
        "LESS",  # < 17
        "NUMBER",  # 5
        "SEMICOLON",  # ;
        "IDENTIFIER",  # b
        "EQUAL",  # =
        "IDENTIFIER",  # temp
        "PLUS",  # +
        "IDENTIFIER",  # b
        "RIGHT_PAREN",  # )
        "LEFT_BRACE",  # {
        "PRINT",  # print
        "IDENTIFIER",  # a
        "SEMICOLON",  # ;
        "IDENTIFIER",  # temp
        "EQUAL",  # =
        "IDENTIFIER",  # a
        "SEMICOLON",  # ;
        "IDENTIFIER",  # a
        "EQUAL",  # =
        "IDENTIFIER",  # b
        "SEMICOLON",  # ;
        "RIGHT_BRACE",  # }
        "EOF",  # }
    ]


def test_semi():
    scanner = get_tokens("print 5!")
    assert scanner.tokens[1].type == TokenType.NUMBER
    assert scanner.tokens[1].literal == 5


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


def test_string():
    scanner = get_tokens('"testing this stuff"')
    assert scanner.tokens[0].type == TokenType.STRING
    assert scanner.tokens[0].literal == "testing this stuff"


def test_identifiers():
    scanner = get_tokens("testing; this; stuff;")
    assert scanner.tokens[0].type == TokenType.IDENTIFIER
    assert scanner.tokens[0].literal == "testing"
    assert scanner.tokens[1].type == TokenType.SEMICOLON

    assert scanner.tokens[2].type == TokenType.THIS
    assert scanner.tokens[2].literal == "this"
    assert scanner.tokens[3].type == TokenType.SEMICOLON

    assert scanner.tokens[4].type == TokenType.IDENTIFIER
    assert scanner.tokens[4].literal == "stuff"
    assert scanner.tokens[5].type == TokenType.SEMICOLON


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

    assert scanner.tokens[0].type == TokenType.BANG_EQUAL
    assert scanner.tokens[1].type == TokenType.EQUAL_EQUAL
    assert scanner.tokens[2].type == TokenType.LESS_EQUAL
    assert scanner.tokens[3].type == TokenType.GREATER_EQUAL


def test_numbers():
    scanner = get_tokens("1; 1.0; 11.01;")

    assert scanner.tokens[0].type == TokenType.NUMBER
    assert str(scanner.tokens[0].literal) == str(1.0)
    assert scanner.tokens[1].type == TokenType.SEMICOLON
    assert scanner.tokens[2].type == TokenType.NUMBER
    assert str(scanner.tokens[2].literal) == str(1.0)
    assert scanner.tokens[3].type == TokenType.SEMICOLON

    assert scanner.tokens[4].type == TokenType.NUMBER
    assert scanner.tokens[4].literal == 11.01
    assert scanner.tokens[5].type == TokenType.SEMICOLON


def test_keywords():
    scanner = get_tokens(
        "and; class; else; false; for; fun; if nil or print return super this true var while"
    )

    assert scanner.tokens[0].type == TokenType.AND
    assert scanner.tokens[0].literal == "and"
    assert scanner.tokens[1].type == TokenType.SEMICOLON

    assert scanner.tokens[2].type == TokenType.CLASS
    assert scanner.tokens[2].literal == "class"
    assert scanner.tokens[3].type == TokenType.SEMICOLON

    assert scanner.tokens[4].type == TokenType.ELSE
    assert scanner.tokens[4].literal == "else"
    assert scanner.tokens[5].type == TokenType.SEMICOLON

    assert scanner.tokens[6].type == TokenType.FALSE
    assert scanner.tokens[6].literal == "false"
    assert scanner.tokens[7].type == TokenType.SEMICOLON

    assert scanner.tokens[8].type == TokenType.FOR
    assert scanner.tokens[8].literal == "for"
    assert scanner.tokens[9].type == TokenType.SEMICOLON

    assert scanner.tokens[10].type == TokenType.FUN
    assert scanner.tokens[10].literal == "fun"
    assert scanner.tokens[11].type == TokenType.SEMICOLON

    assert scanner.tokens[12].type == TokenType.IF
    assert scanner.tokens[12].literal == "if"

    assert scanner.tokens[13].type == TokenType.NIL
    assert scanner.tokens[13].literal == "nil"

    assert scanner.tokens[14].type == TokenType.OR
    assert scanner.tokens[14].literal == "or"

    assert scanner.tokens[15].type == TokenType.PRINT
    assert scanner.tokens[15].literal == "print"

    assert scanner.tokens[16].type == TokenType.RETURN
    assert scanner.tokens[16].literal == "return"

    assert scanner.tokens[17].type == TokenType.SUPER
    assert scanner.tokens[17].literal == "super"

    assert scanner.tokens[18].type == TokenType.THIS
    assert scanner.tokens[18].literal == "this"

    assert scanner.tokens[19].type == TokenType.TRUE
    assert scanner.tokens[19].literal == "true"

    assert scanner.tokens[20].type == TokenType.VAR
    assert scanner.tokens[20].literal == "var"

    assert scanner.tokens[21].type == TokenType.WHILE
    assert scanner.tokens[21].literal == "while"

    assert scanner.tokens[22].type == TokenType.EOF


if __name__ == "__main__":
    test_keywords()
