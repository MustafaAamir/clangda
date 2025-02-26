#pragma once
#include "lexer.h"
#include "lambda.h"

Exp* parse(const char* input);
Exp* parse_expression(Lexer* lexer);
