#ifndef cpplox_compiler_h
#define cpplox_compiler_h

#include "common.h"
#include "scanner.h"
#include "vm.h"

typedef struct {
  Token *current;
  bool hadError;
  bool panicMode;
  Token *previous;
  Chunk * chunk;
} Parser;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Parser * parser, Scanner * scanner);

typedef struct {
  ParseFn* prefix;
  ParseFn*infix;
  Precedence precedence;
} ParseRule;

bool compile(std::string source, Chunk *chunk);

#endif
