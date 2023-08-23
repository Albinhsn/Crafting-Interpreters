

#include "scanner.h"
#include "common.h"
#include <map>

static bool isAtEnd(Scanner *scanner) {
  return scanner->source.size() >= scanner->current;
}

static Token makeToken(TokenType type, Scanner *scanner) {
  Token token;
  token.type = type;
  token.literal =
      scanner->source.substr(scanner->start, scanner->current - scanner->start);
  token.line = scanner->line;

  return token;
}

static Token errorToken(std::string message, Scanner *scanner) {
  Token token;
  token.type = TOKEN_ERROR;
  token.literal = message;
  token.line = scanner->line;

  return token;
}

static char advance(Scanner *scanner) {
  scanner->current++;
  return scanner->source[scanner->current];
}

static bool match(char expected, Scanner *scanner) {
  if (isAtEnd(scanner))
    return false;

  if (scanner->source[scanner->current] != expected) {
    return false;
  }

  scanner->current++;
  return true;
}
static char peek(Scanner *scanner) { return scanner->source[scanner->current]; }

static char peekNext(Scanner *scanner) {
  if (isAtEnd(scanner)) {
    return '\0';
  }
  return scanner->source[scanner->current + 1];
}

static void skipWhitespace(Scanner *scanner) {
  for (;;) {
    char c = peek(scanner);
    switch (c) {
    case ' ':
    case '\r':
    case '\t': {
      advance(scanner);
      break;
    }
    case '\n': {
      scanner->line++;
      advance(scanner);
      break;
    }
    case '/': {
      if (peekNext(scanner) == '/') {
        while (peek(scanner) != '\n' && !isAtEnd(scanner)) {
          advance(scanner);
        }
      } else {
        return;
      }
      break;
    }
    default:
      return;
    }
  }
}

static Token string(Scanner *scanner) {
  while (peek(scanner) != '"' && !isAtEnd(scanner)) {
    if (peek(scanner) == '\n') {
      scanner->line++;
    }
    advance(scanner);
  }

  if (isAtEnd(scanner)) {
    return errorToken("Unterminated string.", scanner);
  }

  advance(scanner);
  return makeToken(TOKEN_STRING, scanner);
}

static bool isDigit(char c) { return c >= '0' && c <= '9'; }

static Token number(Scanner *scanner) {
  while (isDigit(peek(scanner))) {
    advance(scanner);
  }

  if (peek(scanner) == '.' && isDigit(peekNext(scanner))) {
    advance(scanner);
    while (isDigit(peek(scanner))) {
      advance(scanner);
    }
  }

  return makeToken(TOKEN_NUMBER, scanner);
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static TokenType identifierType(Scanner *scanner) {
  std::string name =
      scanner->source.substr(scanner->start, scanner->current - scanner->start);
  std::map<std::string, TokenType> m = {
      {"and", TOKEN_AND},     {"struct", TOKEN_STRUCT}, {"else", TOKEN_ELSE},
      {"if", TOKEN_IF},       {"nil", TOKEN_NIL},       {"or", TOKEN_OR},
      {"print", TOKEN_PRINT}, {"return", TOKEN_RETURN}, {"var", TOKEN_VAR},
      {"while", TOKEN_WHILE}, {"for", TOKEN_FOR},       {"false", TOKEN_FALSE},
      {"true", TOKEN_TRUE},
  };
  return m.count(name) == 0 ? TOKEN_IDENTIFIER : m[name];
}

static Token identifier(Scanner *scanner) {
  while (isAlpha(peek(scanner)) || isDigit(peek(scanner))) {
    advance(scanner);
  }

  return makeToken(identifierType(scanner), scanner);
}

Token scanToken(Scanner *scanner) {
  skipWhitespace(scanner);
  if (isAtEnd(scanner)) {
    return makeToken(TOKEN_EOF, scanner);
  }

  char c = advance(scanner);
  if (isAlpha(c)) {
    return identifier(scanner);
  }
  if (isDigit(c)) {
    return number(scanner);
  }

  switch (c) {
  case '(': {
    return makeToken(TOKEN_LEFT_PAREN, scanner);
  }
  case ')': {
    return makeToken(TOKEN_RIGHT_PAREN, scanner);
  }
  case '{': {
    return makeToken(TOKEN_LEFT_BRACE, scanner);
  }
  case '}': {
    return makeToken(TOKEN_RIGHT_BRACE, scanner);
  }
  case ';': {
    return makeToken(TOKEN_SEMICOLON, scanner);
  }
  case ',': {
    return makeToken(TOKEN_COMMA, scanner);
  }
  case '.': {
    return makeToken(TOKEN_DOT, scanner);
  }
  case '-': {
    return makeToken(TOKEN_MINUS, scanner);
  }
  case '+': {
    return makeToken(TOKEN_PLUS, scanner);
  }
  case '/': {
    return makeToken(TOKEN_SLASH, scanner);
  }
  case '*': {
    return makeToken(TOKEN_STAR, scanner);
  }
  case '!':
    return makeToken(match('=', scanner) ? TOKEN_BANG_EQUAL : TOKEN_BANG,
                     scanner);
  case '=':
    return makeToken(match('=', scanner) ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL,
                     scanner);
  case '<':
    return makeToken(match('=', scanner) ? TOKEN_LESS_EQUAL : TOKEN_LESS,
                     scanner);
  case '>':
    return makeToken(match('=', scanner) ? TOKEN_GREATER_EQUAL : TOKEN_GREATER,
                     scanner);
  case '"':
    return string(scanner);
  }

  return errorToken("Unexpected character.", scanner);
}

Scanner *initScanner(std::string source) {
  Scanner *scanner = new Scanner();
  scanner->source = source;
  scanner->line = 1;
  scanner->current = 0;

  return scanner;
}
