#include "scanner.h"
#include "debug.h"

#include <iostream>
#include <map>
#include <stdexcept>

Scanner *initScanner(std::string source) {
  Scanner *scanner = new Scanner();
  scanner->source = source;
  scanner->current = 0;
  scanner->line = 1;
  scanner->indent = 0;

  return scanner;
}

void resetScanner(Scanner *scanner) {
  scanner->current = 0;
  scanner->indent = 0;
  scanner->line = 1;
  scanner->source = "";
}

static char getCurrent(Scanner *scanner) {
  return scanner->source[scanner->current];
}
static char getPrevious(Scanner *scanner) {
  return scanner->source[scanner->current - 1];
}

static Token *makeToken(Scanner *scanner, std::string literal, TokenType type) {
  Token *token = new Token();
  token->literal = literal;
  token->type = type;
  token->line = scanner->line;
  token->indent = scanner->indent;
#ifdef PRINT_TOKENS
  debugToken(token);
#endif
  return token;
}
static bool isAtEnd(Scanner *scanner) {
  return scanner->source[scanner->current] == '\0';
}

static void advance(Scanner *scanner) { scanner->current++; }

static bool match(Scanner *scanner, char needle) {
  if (!isAtEnd(scanner) && scanner->source[scanner->current] == needle) {
    advance(scanner);
    return true;
  }
  return false;
}
static bool isDigit(char c) { return '0' <= c && '9' >= c; }

static Token *parseNumber(Scanner *scanner) {
  int current = scanner->current - 1;
  while (!isAtEnd(scanner) && isDigit(getCurrent(scanner))) {
    advance(scanner);
  }
  if (getCurrent(scanner) == '.') {
    advance(scanner);
  }
  while (!isAtEnd(scanner) && isDigit(getCurrent(scanner))) {
    advance(scanner);
  }
  std::string literal =
      scanner->source.substr(current, scanner->current - current);
  TokenType type = TOKEN_NUMBER;
  return makeToken(scanner, literal, type);
}
static TokenType isKeyword(std::string literal) {
  std::map<std::string, TokenType> m{
      {"struct", TOKEN_STRUCT}, {"else", TOKEN_ELSE}, {"false", TOKEN_FALSE},
      {"for", TOKEN_FOR},       {"if", TOKEN_IF},     {"nil", TOKEN_NIL},
      {"return", TOKEN_RETURN}, {"true", TOKEN_TRUE}, {"while", TOKEN_WHILE},
      {"print", TOKEN_PRINT},   {"var", TOKEN_VAR},   {"fun", TOKEN_FUN}};
  if (m.count(literal)) {
    return m[literal];
  }
  return TOKEN_IDENTIFIER;
}

static bool isAlpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static Token *parseIdentifier(Scanner *scanner) {
  int current = scanner->current - 1;
  while (!isAtEnd(scanner) && isAlpha(getCurrent(scanner))) {
    advance(scanner);
  }
  std::string literal =
      scanner->source.substr(current, scanner->current - current);
  TokenType type = isKeyword(literal);
  return makeToken(scanner, literal, type);
}

static Token *parseString(Scanner *scanner) {
  int current = scanner->current;
  while (!isAtEnd(scanner) && getCurrent(scanner) != '"' &&
         getCurrent(scanner) != '\n') {
    advance(scanner);
  }

  if (isAtEnd(scanner)) {
    throw std::invalid_argument("Hit eof with unterminated string.");
  }
  std::string literal =
      scanner->source.substr(current, scanner->current - current);
  TokenType type = TOKEN_STRING;
  scanner->current++;
  return makeToken(scanner, literal, type);
}

void skipWhitespace(Scanner *scanner) {
  for (;;) {
    if (isAtEnd(scanner)) {
      return;
    }
    char c = getCurrent(scanner);
    switch (c) {
    case '/': {
      if (match(scanner, '/')) {
        while (!isAtEnd(scanner) and !match(scanner, '\n')) {
          advance(scanner);
        }
        scanner->line++;
      }
      return;
    }
    case ' ': {
      scanner->indent++;
      break;
    }
    case '\n': {
      scanner->line++;
      scanner->indent = 0;
      break;
    }
    case '\t': {
      scanner->indent += 4;
      break;
    }
    default:
      return;
    }
    scanner->current++;
  }
}

Token *scanToken(Scanner *scanner) {
  skipWhitespace(scanner);
  if (isAtEnd(scanner)) {
    return makeToken(scanner, "EOF", TOKEN_EOF);
  }
  scanner->current++;
  char c = getPrevious(scanner);
  if (isDigit(c)) {
    return parseNumber(scanner);
  }
  if (isAlpha(c)) {
    return parseIdentifier(scanner);
  }
  switch (c) {
  case '"': {
    return parseString(scanner);
  }
  case '(': {
    return makeToken(scanner, "(", TOKEN_LEFT_PAREN);
  }
  case ')': {
    return makeToken(scanner, ")", TOKEN_RIGHT_PAREN);
  }
  case '{': {
    return makeToken(scanner, "{", TOKEN_LEFT_BRACE);
  }
  case '}': {
    return makeToken(scanner, "}", TOKEN_RIGHT_BRACE);
  }
  case ';': {
    return makeToken(scanner, ";", TOKEN_SEMICOLON);
  }
  case ',': {
    return makeToken(scanner, ",", TOKEN_COMMA);
  }
  case '.': {
    return makeToken(scanner, ".", TOKEN_DOT);
  }
  case '+': {
    return makeToken(scanner, "+", TOKEN_PLUS);
  }
  case '*': {
    return makeToken(scanner, "*", TOKEN_STAR);
  }
    // case '?': {
    //   return makeToken(scanner, "?", TOKEN_QUESTION);
    // }
    // case ':': {
    //   return makeToken(scanner, ":", TOKEN_COLON);
    // }
  case '!': {
    if (match(scanner, '=')) {
      return makeToken(scanner, "!=", TOKEN_BANG_EQUAL);
    }
    return makeToken(scanner, "!", TOKEN_BANG);
  }
  case '=': {
    if (match(scanner, '=')) {
      return makeToken(scanner, "==", TOKEN_EQUAL_EQUAL);
    }
    return makeToken(scanner, "=", TOKEN_EQUAL);
  }
  case '<': {
    if (match(scanner, '=')) {
      return makeToken(scanner, "<=", TOKEN_LESS_EQUAL);
    }
    return makeToken(scanner, "<", TOKEN_LESS);
  }
  case '>': {
    if (match(scanner, '=')) {
      return makeToken(scanner, ">=", TOKEN_GREATER_EQUAL);
    }
    return makeToken(scanner, ">", TOKEN_GREATER);
  }
  case '-': {
    if (match(scanner, '>')) {
      return makeToken(scanner, "->", TOKEN_ARROW);
    }
    return makeToken(scanner, "-", TOKEN_MINUS);
  }
  case '/': {
    return makeToken(scanner, "/", TOKEN_SLASH);
  }
  default:
    std::string exception = "Unknown characther '";
    exception.push_back(c);
    exception.append("'.");
    throw std::invalid_argument(exception);
  }
}
