#pragma once
#include "lambda.h"
#include "lexer.h"

Exp *parse(const char *input);
Exp *parse_expression(Lexer *lexer);
