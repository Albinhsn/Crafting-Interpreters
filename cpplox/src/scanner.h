#ifndef SCANNER_H
#define SCANNER_H

#include "common.h"

typedef enum {

  // Single-character tokens.
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_LEFT_BRACKET,  // [
  TOKEN_RIGHT_BRACKET, //]
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SEMICOLON,
  TOKEN_SLASH,
  TOKEN_STAR,
  // TOKEN_COLON,    // :
  // TOKEN_QUESTION, // ?

  // One or two character tokens.
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,
  TOKEN_ARROW, // ->

  // Literals.
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_NUMBER,

  // Datatypes
  // TOKEN_INT,   // int
  // TOKEN_FLOAT, // int
  // TOKEN_STR,   // str
  // TOKEN_BOOL,  // bool
  // TOKEN_LIST,  // []
  // TOKEN_MAP,   // {}

  // Keywords.
  TOKEN_STRUCT,
  TOKEN_PRINT,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_FUN,
  TOKEN_IF,
  TOKEN_NIL,
  TOKEN_RETURN,
  TOKEN_TRUE,
  TOKEN_WHILE,
  TOKEN_AND,
  TOKEN_OR,
  TOKEN_VAR,
  TOKEN_ERROR,

  TOKEN_EOF
} TokenType;

typedef struct {
  std::string literal;
  int line;
  int indent;
  TokenType type;
} Token;

typedef struct {
  std::string source;
  int current;
  int line;
  int indent;
} Scanner;

void resetScanner(Scanner *scanner);
Scanner *initScanner(std::string source);
Token *scanToken(Scanner *scanner);
#endif
