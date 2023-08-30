

#include "compiler.h"
#include "chunk.h"
#include "common.h"
#include "object.h"
#include "scanner.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <stdexcept>

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

static void statement(Compiler *compiler, Parser *parser, Scanner *scanner);
static void declaration(Compiler *compiler, Parser *parser, Scanner *scanner);

static void prefixRule(Compiler *compiler, Parser *parser, Scanner *scanner,
                       TokenType type, bool canAssign);
static void infixRule(Compiler *compiler, Parser *parser, Scanner *scanner,
                      TokenType type, bool canAssign);
static Parser *initParser() {
  Parser *parser = new Parser();
  parser->current = NULL;
  parser->previous = NULL;
  parser->panicMode = false;
  parser->hadError = false;

  return parser;
}

static Chunk *currentChunk(Compiler *compiler) {
  return compiler->function->chunk;
}

static Compiler *initCompiler(Compiler *current, Parser *parser,
                              FunctionType type) {
  Compiler *compiler = new Compiler();
  compiler->enclosing = current;
  compiler->locals = std::vector<Local>();
  compiler->scopeDepth = 0;
  compiler->function = newFunction();
  compiler->type = type;

  if (type != TYPE_SCRIPT) {
    compiler->function->name = new ObjString();
    compiler->function->name->chars = parser->previous->literal;
  }

  Local *local = new Local();
  local->depth = 0;
  local->name.type = TOKEN_NIL;
  local->name.literal = "compiler";
  return compiler;
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

static bool check(Parser *parser, TokenType type) {
  return parser->current->type == type;
}

static bool match(Parser *parser, Scanner *scanner, TokenType type) {
  if (!check(parser, type)) {
    return false;
  }
  advance(parser, scanner);
  return true;
}

static void emitByte(Compiler *compiler, Parser *parser, uint8_t byte) {
  writeChunk(currentChunk(compiler), byte, parser->previous->line);
}

static void emitBytes(Compiler *compiler, Parser *parser, uint8_t byte1,
                      uint8_t byte2) {
  emitByte(compiler, parser, byte1);
  emitByte(compiler, parser, byte2);
}

static void emitLoop(Compiler *compiler, Parser *parser, int loopStart) {
  emitByte(compiler, parser, OP_LOOP);

  int offset = currentChunk(compiler)->code.size() - loopStart + 2;
  if (offset > UINT16_MAX) {
    error(parser, "Loop body too large.");
  }

  emitByte(compiler, parser, (offset >> 8 & 0xff));
  emitByte(compiler, parser, offset & 0xff);
}

static int emitJump(Compiler *compiler, Parser *parser, uint8_t instruction) {
  emitByte(compiler, parser, instruction);
  emitByte(compiler, parser, 0xff);
  emitByte(compiler, parser, 0xff);

  return currentChunk(compiler)->code.size() - 2;
}

static void emitReturn(Compiler *compiler, Parser *parser) {
  emitByte(compiler, parser, OP_NIL);
  emitByte(compiler, parser, OP_RETURN);
}

static uint8_t makeConstant(Compiler *compiler, Parser *parser, Value value) {
  int constant = addConstant(currentChunk(compiler), value);

  if (constant > UINT8_MAX) {
    error(parser, "Too many constants in one chunk.");
    return 0;
  }
  return (uint8_t)constant;
}

static ObjFunction *endCompiler(Compiler *compiler, Parser *parser) {
#ifdef DEBUG_PRINT_CODE
  if (!parser->hadError) {
    disassembleChunk(currentChunk(compiler),
                     compiler->function->name != NULL
                         ? compiler->function->name->chars
                         : "<script>");
  }
#endif
  emitReturn(compiler, parser);
  return compiler->function;
}

static Precedence getPrecedence(TokenType type) {
  switch (type) {
  case TOKEN_LEFT_PAREN:
    return PREC_CALL;
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
  case TOKEN_AND:
    return PREC_AND;
  case TOKEN_OR:
    return PREC_OR;
  default:
    return PREC_NONE;
  }
}

static void parsePrecedence(Compiler *compiler, Parser *parser,
                            Scanner *scanner, Precedence precedence) {
  advance(parser, scanner);
  bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(compiler, parser, scanner, parser->previous->type, canAssign);
  while (precedence <= getPrecedence(parser->current->type)) {
    advance(parser, scanner);
    infixRule(compiler, parser, scanner, parser->previous->type, canAssign);
  }
  if (canAssign && match(parser, scanner, TOKEN_EQUAL)) {
    error(parser, "Invalid assignment target.");
  }
}

static uint8_t identifierConstant(Compiler *compiler, Parser *parser) {
  return makeConstant(compiler, parser,
                      OBJ_VAL(copyString(parser->previous->literal)));
}

static void addLocal(Compiler *compiler, Token name) {
  Local local;
  local.name = name;
  local.depth = -1;
  compiler->locals.push_back(local);
}

static bool identifiersEqual(Token *a, Token *b) {
  return a->literal == b->literal;
}

static int resolveLocal(Compiler *compiler, Parser *parser) {
  for (int i = compiler->locals.size() - 1; i >= 0; i--) {
    Local local = compiler->locals[i];
    if (identifiersEqual(&local.name, parser->previous)) {
      if (local.depth == -1) {
        error(parser, "Can't read local variable in its own initializer.");
      }
      return i;
    }
  }
  return -1;
}

static void declareVariable(Compiler *compiler, Parser *parser) {
  if (compiler->scopeDepth == 0) {
    return;
  }

  Token *name = parser->previous;
  for (int i = compiler->locals.size() - 1; i >= 0; i--) {
    Local local = compiler->locals[i];
    if (local.depth != -1 && local.depth < compiler->scopeDepth) {
      break;
    }

    if (identifiersEqual(name, &local.name)) {
      error(parser, "Already a variable with this name in this scope.");
    }
  }

  addLocal(compiler, *name);
}

static uint8_t parseVariable(Compiler *compiler, Parser *parser,
                             Scanner *scanner, const char *errorMessage) {
  consume(parser, scanner, TOKEN_IDENTIFIER, errorMessage);

  declareVariable(compiler, parser);
  if (compiler->scopeDepth > 0) {
    return 0;
  }

  return identifierConstant(compiler, parser);
}

static void patchJump(Compiler *compiler, Parser *parser, int offset) {
  // -2 to adjust for the bytecode for the jump offset itself.

  int jump = currentChunk(compiler)->code.size() - offset - 2;
  if (jump > UINT16_MAX) {
    error(parser, "Too much code to jump over.");
  }
  currentChunk(compiler)->code[offset] = (jump >> 8) & 0xff;
  currentChunk(compiler)->code[offset + 1] = jump & 0xff;
}
static void expression(Compiler *compiler, Parser *parser, Scanner *scanner) {
  parsePrecedence(compiler, parser, scanner, PREC_ASSIGNMENT);
}

static void markInitialized(Compiler *compiler) {
  if (compiler->scopeDepth == 0) {
    return;
  }
  compiler->locals[compiler->locals.size() - 1].depth = compiler->scopeDepth;
}

static void defineVariable(Compiler *compiler, Parser *parser, uint8_t global) {
  if (compiler->scopeDepth > 0) {
    markInitialized(compiler);
    return;
  }

  emitBytes(compiler, parser, OP_DEFINE_GLOBAL, global);
}

static uint8_t argumentList(Compiler *compiler, Parser *parser,
                            Scanner *scanner) {
  uint8_t argCount = 0;
  if (!check(parser, TOKEN_RIGHT_PAREN)) {
    do {
      expression(compiler, parser, scanner);
      if (argCount == 255) {
        error(parser, "Can't have more than 255 arguments.");
      }
      argCount++;
    } while (match(parser, scanner, TOKEN_COMMA));
  }
  consume(parser, scanner, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  return argCount;
}

static void and_(Compiler *compiler, Parser *parser, Scanner *scanner,
                 bool canAssign) {
  int endJump = emitJump(compiler, parser, OP_JUMP_IF_FALSE);

  emitByte(compiler, parser, OP_POP);
  parsePrecedence(compiler, parser, scanner, PREC_AND);

  patchJump(compiler, parser, endJump);
}

static void or_(Compiler *compiler, Parser *parser, Scanner *scanner,
                bool canAssign) {
  int elseJump = emitJump(compiler, parser, OP_JUMP_IF_FALSE);
  int endJump = emitJump(compiler, parser, OP_JUMP);

  patchJump(compiler, parser, elseJump);
  emitByte(compiler, parser, OP_POP);

  parsePrecedence(compiler, parser, scanner, PREC_OR);
  patchJump(compiler, parser, endJump);
}

static void varDeclaration(Compiler *compiler, Parser *parser,
                           Scanner *scanner) {
  uint8_t global =
      parseVariable(compiler, parser, scanner, "Expect variable name.");

  if (match(parser, scanner, TOKEN_EQUAL)) {
    expression(compiler, parser, scanner);
  } else {
    emitByte(compiler, parser, OP_NIL);
  }
  consume(parser, scanner, TOKEN_SEMICOLON,
          "Expect ';' after variable declaration");

  defineVariable(compiler, parser, global);
}

static void expressionStatement(Compiler *compiler, Parser *parser,
                                Scanner *scanner) {
  expression(compiler, parser, scanner);
  consume(parser, scanner, TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(compiler, parser, OP_POP);
}

static void beginScope(Compiler *compiler) { compiler->scopeDepth++; }
static void endScope(Compiler *compiler, Parser *parser) {
  compiler->scopeDepth--;
  while (compiler->locals.size() > 0 &&
         compiler->locals[compiler->locals.size() - 1].depth >
             compiler->scopeDepth) {
    emitByte(compiler, parser, OP_POP);
    compiler->locals.pop_back();
  }
}

static void forStatement(Compiler *compiler, Parser *parser, Scanner *scanner) {
  beginScope(compiler);
  consume(parser, scanner, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
  if (match(parser, scanner, TOKEN_SEMICOLON)) {
    //
  } else if (match(parser, scanner, TOKEN_VAR)) {
    varDeclaration(compiler, parser, scanner);
  } else {
    expressionStatement(compiler, parser, scanner);
  }

  int loopStart = currentChunk(compiler)->code.size();
  int exitJump = -1;
  if (!match(parser, scanner, TOKEN_SEMICOLON)) {
    expression(compiler, parser, scanner);
    consume(parser, scanner, TOKEN_SEMICOLON,
            "Expect ';' after loop condition");

    exitJump = emitJump(compiler, parser, OP_JUMP_IF_FALSE);
    emitByte(compiler, parser, OP_POP);
  }

  if (!match(parser, scanner, TOKEN_RIGHT_PAREN)) {
    int bodyJump = emitJump(compiler, parser, OP_JUMP);
    int incrementStart = currentChunk(compiler)->code.size();

    expression(compiler, parser, scanner);
    emitByte(compiler, parser, OP_POP);
    consume(parser, scanner, TOKEN_RIGHT_PAREN,
            "Expect ')' after for clauses.");

    emitLoop(compiler, parser, loopStart);
    loopStart = incrementStart;
    patchJump(compiler, parser, bodyJump);
  }

  statement(compiler, parser, scanner);
  emitLoop(compiler, parser, loopStart);

  if (exitJump != -1) {
    patchJump(compiler, parser, exitJump);
    emitByte(compiler, parser, OP_POP);
  }

  endScope(compiler, parser);
}

static void ifStatement(Compiler *compiler, Parser *parser, Scanner *scanner) {
  consume(parser, scanner, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  expression(compiler, parser, scanner);
  consume(parser, scanner, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int thenJump = emitJump(compiler, parser, OP_JUMP_IF_FALSE);
  emitByte(compiler, parser, OP_POP);
  statement(compiler, parser, scanner);

  int elseJump = emitJump(compiler, parser, OP_JUMP);

  patchJump(compiler, parser, thenJump);
  emitByte(compiler, parser, OP_POP);

  if (match(parser, scanner, TOKEN_ELSE)) {
    statement(compiler, parser, scanner);
  }
  patchJump(compiler, parser, elseJump);
}

static void printStatement(Compiler *compiler, Parser *parser,
                           Scanner *scanner) {
  expression(compiler, parser, scanner);
  consume(parser, scanner, TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(compiler, parser, OP_PRINT);
}

static void returnStatement(Compiler *compiler, Parser *parser,
                            Scanner *scanner) {
  if (compiler->type == TYPE_SCRIPT) {
    error(parser, "Can't return from top-level code");
  }

  if (match(parser, scanner, TOKEN_SEMICOLON)) {
    emitReturn(compiler, parser);
  } else {
    expression(compiler, parser, scanner);
    consume(parser, scanner, TOKEN_SEMICOLON, "Expect ';' after return value");
    emitByte(compiler, parser, OP_RETURN);
  }
}

static void whileStatement(Compiler *compiler, Parser *parser,
                           Scanner *scanner) {
  int loopStart = currentChunk(compiler)->code.size();
  consume(parser, scanner, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  expression(compiler, parser, scanner);
  consume(parser, scanner, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int exitJump = emitJump(compiler, parser, OP_JUMP_IF_FALSE);
  emitByte(compiler, parser, OP_POP);
  statement(compiler, parser, scanner);
  emitLoop(compiler, parser, loopStart);

  patchJump(compiler, parser, exitJump);
  emitByte(compiler, parser, OP_POP);
}

static void synchronize(Parser *parser, Scanner *scanner) {
  parser->panicMode = false;

  while (parser->current->type != TOKEN_EOF) {
    if (parser->previous->type == TOKEN_SEMICOLON) {
      return;
    }
    switch (parser->current->type) {
    case TOKEN_STRUCT:
    case TOKEN_FUN:
    case TOKEN_VAR:
    case TOKEN_FOR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_PRINT:
    case TOKEN_RETURN:
      return;
    default: {
      // Do nothing.
    }
    }

    advance(parser, scanner);
  }
}

static void binary(Compiler *compiler, Parser *parser, Scanner *scanner) {
  TokenType operatorType = parser->previous->type;

  parsePrecedence(compiler, parser, scanner,
                  (Precedence)(getPrecedence(operatorType) + 1));

  switch (operatorType) {
  case TOKEN_BANG_EQUAL: {
    emitBytes(compiler, parser, OP_EQUAL, OP_NOT);
    break;
  }
  case TOKEN_EQUAL_EQUAL: {
    emitByte(compiler, parser, OP_EQUAL);
    break;
  }
  case TOKEN_GREATER: {
    emitByte(compiler, parser, OP_GREATER);
    break;
  }
  case TOKEN_GREATER_EQUAL: {
    emitBytes(compiler, parser, OP_LESS, OP_NOT);
    break;
  }
  case TOKEN_LESS: {
    emitByte(compiler, parser, OP_LESS);
    break;
  }
  case TOKEN_LESS_EQUAL: {
    emitBytes(compiler, parser, OP_GREATER, OP_NOT);
    break;
  }
  case TOKEN_PLUS: {
    emitByte(compiler, parser, OP_ADD);
    break;
  }
  case TOKEN_MINUS: {
    emitByte(compiler, parser, OP_SUBTRACT);
    break;
  }
  case TOKEN_STAR: {
    emitByte(compiler, parser, OP_MULTIPLY);
    break;
  }
  case TOKEN_SLASH: {
    emitByte(compiler, parser, OP_DIVIDE);
    break;
  }
  default: {
    return;
  }
  }
}

static void call(Compiler *compiler, Parser *parser, Scanner *scanner) {
  uint8_t argCount = argumentList(compiler, parser, scanner);
  emitBytes(compiler, parser, OP_CALL, argCount);
}

static void grouping(Compiler *compiler, Parser *parser, Scanner *scanner) {
  expression(compiler, parser, scanner);
  consume(parser, scanner, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary(Compiler *compiler, Parser *parser, Scanner *scanner) {
  TokenType operatorType = parser->previous->type;
  parsePrecedence(compiler, parser, scanner, PREC_UNARY);

  switch (operatorType) {
  case TOKEN_MINUS: {
    emitByte(compiler, parser, OP_NEGATE);
    break;
  }
  case TOKEN_BANG: {
    emitByte(compiler, parser, OP_NOT);
    break;
  }
  default: {
    return;
  }
  }
}

static void emitConstant(Compiler *compiler, Parser *parser, Value value) {
  emitBytes(compiler, parser, OP_CONSTANT,
            makeConstant(compiler, parser, value));
}

static void number(Compiler *compiler, Parser *parser, Scanner *scanner) {
  double value = std::stod(parser->previous->literal);
  emitConstant(compiler, parser, NUMBER_VAL(value));
}

static void string(Compiler *compiler, Parser *parser, Scanner *scanner) {
  emitConstant(compiler, parser,
               OBJ_VAL(copyString(parser->previous->literal)));
}

static void namedVariable(Compiler *compiler, Parser *parser, Scanner *scanner,
                          bool canAssign) {

  uint8_t getOp, setOp;
  int arg = resolveLocal(compiler, parser);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else {
    arg = identifierConstant(compiler, parser);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(parser, scanner, TOKEN_EQUAL)) {
    expression(compiler, parser, scanner);
    emitBytes(compiler, parser, setOp, (uint8_t)arg);
  } else {
    emitBytes(compiler, parser, getOp, (uint8_t)arg);
  }
}

static void variable(Compiler *compiler, Parser *parser, Scanner *scanner,
                     bool canAssign) {
  namedVariable(compiler, parser, scanner, canAssign);
}

static void literal(Compiler *compiler, Parser *parser, Scanner *scanner) {
  switch (parser->previous->type) {
  case TOKEN_FALSE: {
    emitByte(compiler, parser, OP_FALSE);
    break;
  }
  case TOKEN_NIL: {
    emitByte(compiler, parser, OP_NIL);
    break;
  }
  case TOKEN_TRUE: {
    emitByte(compiler, parser, OP_TRUE);
    break;
  }
  default: {
    return;
  }
  }
}

static void prefixRule(Compiler *compiler, Parser *parser, Scanner *scanner,
                       TokenType type, bool canAssign) {
  switch (type) {
  case TOKEN_LEFT_PAREN: {
    grouping(compiler, parser, scanner);
    break;
  }
  case TOKEN_MINUS: {
    unary(compiler, parser, scanner);
    break;
  }
  case TOKEN_STRING: {
    string(compiler, parser, scanner);
    break;
  }
  case TOKEN_NUMBER: {
    number(compiler, parser, scanner);
    break;
  }
  case TOKEN_FALSE: {
    literal(compiler, parser, scanner);
    break;
  }
  case TOKEN_TRUE: {
    literal(compiler, parser, scanner);
    break;
  }
  case TOKEN_NIL: {
    literal(compiler, parser, scanner);
    break;
  }
  case TOKEN_BANG: {
    unary(compiler, parser, scanner);
    break;
  }
  case TOKEN_IDENTIFIER: {
    variable(compiler, parser, scanner, canAssign);
  }
  default: {
    break;
  }
  }
}
static void infixRule(Compiler *compiler, Parser *parser, Scanner *scanner,
                      TokenType type, bool canAssign) {
  switch (type) {
  case TOKEN_LEFT_PAREN: {
    call(compiler, parser, scanner);
    break;
  }
  case TOKEN_MINUS: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_PLUS: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_STAR: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_SLASH: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_BANG_EQUAL: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_EQUAL_EQUAL: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_GREATER: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_GREATER_EQUAL: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_LESS: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_LESS_EQUAL: {
    binary(compiler, parser, scanner);
    break;
  }
  case TOKEN_AND: {
    and_(compiler, parser, scanner, canAssign);
  }
  case TOKEN_OR: {
    or_(compiler, parser, scanner, canAssign);
  }
  default: {
    break;
  }
  }
}

static void block(Compiler *compiler, Parser *parser, Scanner *scanner) {
  while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
    declaration(compiler, parser, scanner);
  }

  consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(Compiler *current, Parser *parser, Scanner *scanner,
                     FunctionType type) {
  Compiler *compiler = initCompiler(current, parser, type);
  beginScope(compiler);
  consume(parser, scanner, TOKEN_LEFT_PAREN, "Expect '(' after function name.");
  if (!check(parser, TOKEN_RIGHT_PAREN)) {
    do {
      compiler->function->arity++;
      if (compiler->function->arity > 255) {
        errorAtCurrent(parser, "Can't at have more than 255 parameters.");
      }
      uint8_t constant =
          parseVariable(compiler, parser, scanner, "Expect parameter name.");
      defineVariable(compiler, parser, constant);
    } while (match(parser, scanner, TOKEN_COMMA));
  }
  consume(parser, scanner, TOKEN_RIGHT_PAREN,
          "Expect ')' after last function param.");
  consume(parser, scanner, TOKEN_LEFT_BRACE,
          "Expect '{' after function params.");
  block(compiler, parser, scanner);
  ObjFunction *function = endCompiler(compiler, parser);
  compiler = compiler->enclosing;
  emitBytes(compiler, parser, OP_CONSTANT,
            makeConstant(compiler, parser, OBJ_VAL(function)));
}

static void funDeclaration(Compiler *compiler, Parser *parser,
                           Scanner *scanner) {
  uint8_t global =
      parseVariable(compiler, parser, scanner, "Expect function name");
  markInitialized(compiler);
  function(compiler, parser, scanner, TYPE_FUNCTION);
  defineVariable(compiler, parser, global);
}

static void statement(Compiler *compiler, Parser *parser, Scanner *scanner) {
  if (match(parser, scanner, TOKEN_PRINT)) {
    printStatement(compiler, parser, scanner);
  } else if (match(parser, scanner, TOKEN_FOR)) {
    forStatement(compiler, parser, scanner);
  } else if (match(parser, scanner, TOKEN_IF)) {
    ifStatement(compiler, parser, scanner);
  } else if (match(parser, scanner, TOKEN_RETURN)) {
    returnStatement(compiler, parser, scanner);
  } else if (match(parser, scanner, TOKEN_WHILE)) {
    whileStatement(compiler, parser, scanner);
  } else if (match(parser, scanner, TOKEN_LEFT_BRACE)) {
    beginScope(compiler);
    block(compiler, parser, scanner);
    endScope(compiler, parser);
  } else {
    expressionStatement(compiler, parser, scanner);
  }
}

static void declaration(Compiler *compiler, Parser *parser, Scanner *scanner) {
  if (match(parser, scanner, TOKEN_FUN)) {
    funDeclaration(compiler, parser, scanner);
  } else if (match(parser, scanner, TOKEN_VAR)) {
    varDeclaration(compiler, parser, scanner);
  } else {

    statement(compiler, parser, scanner);
  }

  if (parser->panicMode) {
    synchronize(parser, scanner);
  }
}

ObjFunction *compile(std::string source, Chunk *chunk) {
  Scanner *scanner = initScanner(source);
  Parser *parser = initParser();
  Compiler *compiler = initCompiler(NULL, parser, TYPE_SCRIPT);

  advance(parser, scanner);
  while (!match(parser, scanner, TOKEN_EOF)) {
    declaration(compiler, parser, scanner);
  }
  return parser->hadError ? NULL : endCompiler(compiler, parser);
}
