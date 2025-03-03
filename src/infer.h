#pragma once
#include "lambda.h"
#include "types.h"

Type *infer(Exp *exp, TypeEnv *env);
TypeEnv *init_standard_type_env();
