#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to check token type and advance
static void expect(Lexer *lexer, TokenType type) {
  if (lexer->current.type == type) {
    lexer_next(lexer);
  } else {
    fprintf(stdout, "%*s\n", lexer->position, "^");
    fprintf(stderr, "Syntax error: expected token type %d, got %d\n", type,
            lexer->current.type);
    exit(1);
  }
}

// Forward declarations for recursive parsing
static Exp *parse_expr(Lexer *lexer);
static Exp *parse_atom(Lexer *lexer);
static Exp *parse_application(Lexer *lexer);

// Parse a lambda expression (λx.e)
static Exp *parse_lambda(Lexer *lexer) {
  expect(lexer, TOKEN_LAMBDA);

  // Parse parameter
  if (lexer->current.type != TOKEN_IDENTIFIER) {
    fprintf(stderr, "Syntax error: expected identifier after lambda\n");
    exit(1);
  }

  char *param = strdup(lexer->current.data.identifier);
  lexer_next(lexer);

  // Parse dot
  expect(lexer, TOKEN_DOT);

  // Parse body
  Exp *body = parse_expr(lexer);

  return make_lambda(param, body);
}

// Parse a let expression (let x = e1 in e2)
static Exp *parse_let(Lexer *lexer) {
  expect(lexer, TOKEN_LET);

  // Parse variable name
  if (lexer->current.type != TOKEN_IDENTIFIER) {
    fprintf(stderr, "Syntax error: expected identifier after let\n");
    exit(1);
  }

  char *var = strdup(lexer->current.data.identifier);
  lexer_next(lexer);

  // Parse equals sign
  expect(lexer, TOKEN_EQUALS);

  // Parse value expression
  Exp *val = parse_expr(lexer);

  // Parse 'in' keyword
  expect(lexer, TOKEN_IN);

  // Parse body expression
  Exp *body = parse_expr(lexer);

  return make_let(var, val, body);
}

// Parse an atomic expression (literal, variable, or parenthesized expression)
static Exp *parse_atom(Lexer *lexer) {
  switch (lexer->current.type) {
  case TOKEN_INT: {
    int val = lexer->current.data.int_val;
    lexer_next(lexer);
    return make_int(val);
  }

  case TOKEN_TRUE: {
    lexer_next(lexer);
    return make_bool(true);
  }

  case TOKEN_FALSE: {
    lexer_next(lexer);
    return make_bool(false);
  }

  case TOKEN_UNIT: {
    lexer_next(lexer);
    return make_unit();
  }

  case TOKEN_IDENTIFIER: {
    char *name = strdup(lexer->current.data.identifier);
    lexer_next(lexer);
    return make_var(name);
  }

  case TOKEN_LPAREN: {
    lexer_next(lexer);
    Exp *expr = parse_expr(lexer);
    expect(lexer, TOKEN_RPAREN);
    return expr;
  }

  case TOKEN_LAMBDA:
    return parse_lambda(lexer);

  case TOKEN_LET:
    return parse_let(lexer);

  default:
    fprintf(stderr, "%*s\n", lexer->position, "^");
    fprintf(stderr, "Syntax error: unexpected token type %d\n", lexer->current.type);
    exit(1);
  }
}

// Parse function application
static Exp *parse_application(Lexer *lexer) {
  Exp *fn = parse_atom(lexer);

  while (
      lexer->current.type == TOKEN_IDENTIFIER ||
      lexer->current.type == TOKEN_LPAREN || lexer->current.type == TOKEN_INT ||
      lexer->current.type == TOKEN_TRUE || lexer->current.type == TOKEN_FALSE ||
      lexer->current.type == TOKEN_UNIT ||
      lexer->current.type == TOKEN_LAMBDA) {

    Exp *arg = parse_atom(lexer);
    fn = make_apply(fn, arg);
  }

  return fn;
}

// Parse an expression
static Exp *parse_expr(Lexer *lexer) { return parse_application(lexer); }

// Parse the entire input
Exp *parse(const char *input) {
  Lexer *lexer = lexer_init(input);
  Exp *result = parse_expr(lexer);

  if (lexer->current.type != TOKEN_EOF) {
    fprintf(stderr, "Syntax error: unexpected tokens after expression\n");
    exit(1);
  }

  lexer_free(lexer);
  return result;
}
