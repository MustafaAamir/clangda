#pragma once

#include <stdbool.h>
typedef enum {
  TOKEN_EOF,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LAMBDA,
  TOKEN_DOT,
  TOKEN_LET,
  TOKEN_EQUALS,
  TOKEN_IN,
  TOKEN_IDENTIFIER,
  TOKEN_INT,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_UNIT
} TokenType;


typedef struct {
  TokenType type;
  union {
    char *identifier;
    int int_val;
  } data;
} Token;

typedef struct {
  const char *input;
  int position;
  int line;
  int column;
  Token current;
} Lexer;

Lexer *lexer_init(const char *input);
void lexer_next(Lexer *lexer);
void lexer_free(Lexer *lexer);
void token_free(Token *token);
char *string_of_tokentype(TokenType tt);
