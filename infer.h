// infer.h
#ifndef INFER_H
#define INFER_H

#include "lambda.h"
#include "types.h"

// Main type inference function
Type *infer(Exp *exp, TypeEnv *env);

// Initialize the standard environment with primitive types
TypeEnv *init_standard_type_env();

#endif // INFER_H
