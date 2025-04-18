#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Lexer *lexer_init(const char *input) {
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    lexer->input = input;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;

    // Initialize token
    lexer->current.type = TOKEN_EOF;

    // Get the first token
    lexer_next(lexer);

    return lexer;
}

static void skip_whitespace(Lexer *lexer) {
    while (lexer->input[lexer->position] != '\0') {
        if (isspace(lexer->input[lexer->position])) {
            if (lexer->input[lexer->position] == '\n') {
                lexer->line++;
                lexer->column = 1;
            } else {
                lexer->column++;
            }
            lexer->position++;
        } else if (lexer->input[lexer->position] == '#') {
            // Skip comments
            while (lexer->input[lexer->position] != '\0' &&
                   lexer->input[lexer->position] != '\n') {
                lexer->position++;
            }
        } else {
            break;
        }
    }
}
static bool is_id_char(char c) { return isalnum(c) || c == '_' || c == '\''; }

// Get the next token
void lexer_next(Lexer *lexer) {
    // Free previous token if needed
    if (lexer->current.type == TOKEN_IDENTIFIER) {
        free(lexer->current.data.identifier);
    }

    // Skip whitespace and comments
    skip_whitespace(lexer);

    // Check for end of input
    if (lexer->input[lexer->position] == '\0') {
        lexer->current.type = TOKEN_EOF;
        return;
    }

    // Check for single character tokens
    switch (lexer->input[lexer->position]) {
        case '(':
            lexer->current.type = TOKEN_LPAREN;
            lexer->position++;
            lexer->column++;
            return;

        case ')':
            lexer->current.type = TOKEN_RPAREN;
            lexer->position++;
            lexer->column++;
            return;

        case '.':
            lexer->current.type = TOKEN_DOT;
            lexer->position++;
            lexer->column++;
            return;

        case '=':
            lexer->current.type = TOKEN_EQUALS;
            lexer->position++;
            lexer->column++;
            return;

        case '\\':  // Lambda
            lexer->current.type = TOKEN_LAMBDA;
            lexer->position++;
            lexer->column++;
            return;
    }

    // Check for numbers
    if (isdigit(lexer->input[lexer->position])) {
        int value = 0;
        int start_pos = lexer->position;

        while (isdigit(lexer->input[lexer->position])) {
            value = value * 10 + (lexer->input[lexer->position] - '0');
            lexer->position++;
        }

        lexer->current.type = TOKEN_INT;
        lexer->current.data.int_val = value;
        lexer->column += (lexer->position - start_pos);
        return;
    }

    // Check for identifiers and keywords
    if (isalpha(lexer->input[lexer->position]) ||
        lexer->input[lexer->position] == '_') {
        int start_pos = lexer->position;

        while (is_id_char(lexer->input[lexer->position])) {
            lexer->position++;
        }

        int len = lexer->position - start_pos;
        lexer->column += (int)len;

        // Check for keywords
        if (len == 3 && strncmp(lexer->input + start_pos, "let", 3) == 0) {
            lexer->current.type = TOKEN_LET;
        } else if (len == 2 &&
                   strncmp(lexer->input + start_pos, "in", 2) == 0) {
            lexer->current.type = TOKEN_IN;
        } else if (len == 4 &&
                   strncmp(lexer->input + start_pos, "true", 4) == 0) {
            lexer->current.type = TOKEN_TRUE;
        } else if (len == 5 &&
                   strncmp(lexer->input + start_pos, "false", 5) == 0) {
            lexer->current.type = TOKEN_FALSE;
        } else if (len == 4 &&
                   strncmp(lexer->input + start_pos, "unit", 4) == 0) {
            lexer->current.type = TOKEN_UNIT;
        } else {
            // Regular identifier
            lexer->current.type = TOKEN_IDENTIFIER;
            lexer->current.data.identifier = (char *)malloc(len + 1);
            strncpy(lexer->current.data.identifier, lexer->input + start_pos,
                    len);
            lexer->current.data.identifier[len] = '\0';
        }

        return;
    }

    fprintf(stderr, "Unexpected character '%c' at line %d, column %d\n",
            lexer->input[lexer->position], lexer->line, lexer->column);
    exit(1);
}

void lexer_free(Lexer *lexer) {
    if (lexer->current.type == TOKEN_IDENTIFIER) {
        free(lexer->current.data.identifier);
    }
    free(lexer);
}

void token_free(Token *token) {
    if (token->type == TOKEN_IDENTIFIER) {
        free(token->data.identifier);
    }
}

char *string_of_tokentype(TokenType tt) {
    switch (tt) {
        case TOKEN_EOF:
            return "EOF\0";
        case TOKEN_LPAREN:
            return "LPAREN\0";
        case TOKEN_RPAREN:
            return "RPAREN\0";
        case TOKEN_LAMBDA:
            return "LAMBDA\0";
        case TOKEN_DOT:
            return "DOT\0";
        case TOKEN_LET:
            return "LET\0";
        case TOKEN_EQUALS:
            return "EQUALS\0";
        case TOKEN_IN:
            return "IN\0";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER\0";
        case TOKEN_INT:
            return "INT\0";
        case TOKEN_TRUE:
            return "TRUE\0";
        case TOKEN_FALSE:
            return "FALSE\0";
        case TOKEN_UNIT:
            return "UNIT\0";
        default:
            return "NotImplemented";
    }
}
