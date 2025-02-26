// primitives.h
#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "lambda.h"
// Binary primitive operations
Value prim_add(Value a, Value b);
Value prim_subtract(Value a, Value b);
Value prim_multiply(Value a, Value b);
Value prim_equals(Value a, Value b);
Value prim_if(Value cond, Value then_val, Value else_val);

Env *init_standard_env();

#endif
