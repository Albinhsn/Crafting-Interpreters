

#include "compiler.h"
#include "chunk.h"
#include "common.h"
#include "scanner.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <stdexcept>

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

static void prefixRule(Parser *parser, Scanner *scanner, TokenType type);
static void infixRule(Parser *parser, Scanner *scanner, TokenType type);
static Parser *initParser(Chunk *chunk) {
  Parser *parser = new Parser();
  parser->current = NULL;
  parser->previous = NULL;
  parser->panicMode = false;
  parser->hadError = false;
  parser->chunk = chunk;

  return parser;
}

static void errorAt(Parser *parser, std::string message) {
  if (parser->panicMode) {
    return;
  }
  parser->panicMode = true;

  Token *token = parser->current;
  std::fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    std::fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    //
  } else {
    std::fprintf(stderr, " at '%d'", token->line);
  }

  fprintf(stderr, ": %s\n", message.c_str());
  parser->hadError = true;
}

static void error(Parser *parser, std::string message) {
  errorAt(parser, message);
}

static void errorAtCurrent(Parser *parser, std::string message) {
  errorAt(parser, message);
}

static void advance(Parser *parser, Scanner *scanner) {
  parser->previous = parser->current;
  for (;;) {
    parser->current = scanToken(scanner);
    if (parser->current->type != TOKEN_ERROR) {
      break;
    }
    errorAtCurrent(parser, parser->current->literal);
  }
}
static void consume(Parser *parser, Scanner *scanner, TokenType type,
                    std::string message) {
  if (parser->current->type == type) {
    advance(parser, scanner);
    return;
  }

  errorAtCurrent(parser, message);
}

static void emitByte(Parser *parser, uint8_t byte) {
  writeChunk(parser->chunk, byte, parser->previous->line);
}

static void emitBytes(Parser *parser, uint8_t byte1, uint8_t byte2) {
  emitByte(parser, byte1);
  emitByte(parser, byte2);
}

static void emitReturn(Parser *parser) { emitByte(parser, OP_RETURN); }

static uint8_t makeConstant(Parser *parser, Value value) {
  int constant = addConstant(parser->chunk, value);

  if (constant > UINT8_MAX) {
    error(parser, "Too many constants in one chunk.");
    return 0;
  }

  return (uint8_t)constant;
}

static void endCompiler(Parser *parser) {
#ifdef DEBUG_PRINT_CODE
  if (!parser->hadError) {
    disassembleChunk(parser->chunk, "code");
  }
#endif
  emitReturn(parser);
}
static Precedence getPrecedence(TokenType type) {
  switch (type) {
  case TOKEN_MINUS:
    return PREC_TERM;
  case TOKEN_PLUS:
    return PREC_TERM;
  case TOKEN_SLASH:
    return PREC_FACTOR;
  case TOKEN_STAR:
    return PREC_FACTOR;
  case TOKEN_BANG_EQUAL:
    return PREC_EQUALITY;
  case TOKEN_EQUAL_EQUAL:
    return PREC_EQUALITY;
  case TOKEN_GREATER:
    return PREC_COMPARISON;
  case TOKEN_GREATER_EQUAL:
    return PREC_COMPARISON;
  case TOKEN_LESS:
    return PREC_COMPARISON;
  case TOKEN_LESS_EQUAL:
    return PREC_COMPARISON;
  default:
    return PREC_NONE;
  }
}

static void parsePrecedence(Parser *parser, Scanner *scanner,
                            Precedence precedence) {
  advance(parser, scanner);
  prefixRule(parser, scanner, parser->previous->type);
  while (precedence <= getPrecedence(parser->current->type)) {
    advance(parser, scanner);
    std::cout << "Calling infix rule " << parser->current->literal << "\n";
    infixRule(parser, scanner, parser->previous->type);
  }
  std::cout << "Current less than precedence " << parser->current->literal
            << "\n";
}

static void expression(Parser *parser, Scanner *scanner) {
  parsePrecedence(parser, scanner, PREC_ASSIGNMENT);
}

static void binary(Parser *parser, Scanner *scanner) {
  TokenType operatorType = parser->previous->type;

  parsePrecedence(parser, scanner,
                  (Precedence)(getPrecedence(operatorType) + 1));

  switch (operatorType) {
  case TOKEN_BANG_EQUAL: {
    emitBytes(parser, OP_EQUAL, OP_NOT);
    break;
  }
  case TOKEN_EQUAL_EQUAL: {
    emitByte(parser, OP_EQUAL);
    break;
  }
  case TOKEN_GREATER: {
    emitByte(parser, OP_GREATER);
    break;
  }
  case TOKEN_GREATER_EQUAL: {
    emitBytes(parser, OP_LESS, OP_NOT);
    break;
  }
  case TOKEN_LESS: {
    emitByte(parser, OP_LESS);
    break;
  }
  case TOKEN_LESS_EQUAL: {
    emitBytes(parser, OP_GREATER, OP_NOT);
    break;
  }
  case TOKEN_PLUS: {
    emitByte(parser, OP_ADD);
    break;
  }
  case TOKEN_MINUS: {
    emitByte(parser, OP_SUBTRACT);
    break;
  }
  case TOKEN_STAR: {
    emitByte(parser, OP_MULTIPLY);
    break;
  }
  case TOKEN_SLASH: {
    emitByte(parser, OP_DIVIDE);
    break;
  }
  default: {
    return;
  }
  }
}

static void grouping(Parser *parser, Scanner *scanner) {
  expression(parser, scanner);
  consume(parser, scanner, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary(Parser *parser, Scanner *scanner) {
  TokenType operatorType = parser->previous->type;
  parsePrecedence(parser, scanner, PREC_UNARY);

  switch (operatorType) {
  case TOKEN_MINUS: {
    emitByte(parser, OP_NEGATE);
    break;
  }
  case TOKEN_BANG: {
    emitByte(parser, OP_NOT);
    break;
  }
  default: {
    return;
  }
  }
}

static void emitConstant(Parser *parser, Value value) {
  emitBytes(parser, OP_CONSTANT, makeConstant(parser, value));
}

static void number(Parser *parser, Scanner *scanner) {
  double value = std::stod(parser->previous->literal);
  emitConstant(parser, NUMBER_VAL(value));
}

static void literal(Parser *parser, Scanner *scanner) {
  switch (parser->previous->type) {
  case TOKEN_FALSE: {
    emitByte(parser, OP_FALSE);
    break;
  }
  case TOKEN_NIL: {
    emitByte(parser, OP_NIL);
    break;
  }
  case TOKEN_TRUE: {
    emitByte(parser, OP_TRUE);
    break;
  }
  default: {
    return;
  }
  }
}

static void prefixRule(Parser *parser, Scanner *scanner, TokenType type) {
  switch (type) {
  case TOKEN_LEFT_PAREN: {
    grouping(parser, scanner);
    break;
  }
  case TOKEN_MINUS: {
    unary(parser, scanner);
    break;
  }
  case TOKEN_NUMBER: {
    number(parser, scanner);
    break;
  }
  case TOKEN_FALSE: {
    literal(parser, scanner);
    break;
  }
  case TOKEN_TRUE: {
    literal(parser, scanner);
    break;
  }
  case TOKEN_NIL: {
    literal(parser, scanner);
    break;
  }
  case TOKEN_BANG: {
    unary(parser, scanner);
    break;
  }
  default: {
    break;
  }
  }
}
static void infixRule(Parser *parser, Scanner *scanner, TokenType type) {
  switch (type) {
  case TOKEN_MINUS: {
    binary(parser, scanner);
    break;
  }
  case TOKEN_PLUS: {
    binary(parser, scanner);
    break;
  }
  case TOKEN_STAR: {
    binary(parser, scanner);
    break;
  }
  case TOKEN_SLASH: {
    binary(parser, scanner);
    break;
  }
  case TOKEN_BANG_EQUAL: {
    binary(parser, scanner);
    break;
  }
  case TOKEN_EQUAL_EQUAL: {
    binary(parser, scanner);
    break;
  }
  case TOKEN_GREATER: {
    binary(parser, scanner);
    break;
  }
  case TOKEN_GREATER_EQUAL: {
    binary(parser, scanner);
    break;
  }
  case TOKEN_LESS: {
    binary(parser, scanner);
    break;
  }
  case TOKEN_LESS_EQUAL: {
    binary(parser, scanner);
    break;
  }
  default: {
    break;
  }
  }
}

bool compile(std::string source, Chunk *chunk) {
  std::cout << "Compiling - " << source << "\n";
  Scanner *scanner = initScanner(source);
  Parser *parser = initParser(chunk);

  advance(parser, scanner);
  expression(parser, scanner);
  consume(parser, scanner, TOKEN_EOF, "Expect end of expression.");
  endCompiler(parser);

  return !parser->hadError;
}
