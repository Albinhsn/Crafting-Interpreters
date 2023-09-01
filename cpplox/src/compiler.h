#ifndef cpplox_compiler_h
#define cpplox_compiler_h

#include "common.h"
#include "object.h"
#include "scanner.h"
#include "vm.h"

typedef struct {
  Token *current;
  bool hadError;
  bool panicMode;
  Token *previous;
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
  PREC_CALL,       // . (), []
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Parser *parser, Scanner *scanner);

typedef struct {
  ParseFn *prefix;
  ParseFn *infix;
  Precedence precedence;
} ParseRule;

typedef struct {
  Token name;
  int depth;
} Local;

typedef enum { TYPE_FUNCTION, TYPE_SCRIPT } FunctionType;

typedef struct Compiler {
  struct Compiler *enclosing;
  ObjFunction *function;
  FunctionType type;
  std::vector<Local> locals;
  int scopeDepth;
} Compiler;

Compiler * compile(std::string source);

#endif
