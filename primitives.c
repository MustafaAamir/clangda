#include "primitives.h"
#include <stdio.h>
#include <stdlib.h>

extern void throw_error(const char *message);

Value prim_add(Value a, Value b) {
  if (a.type != VAL_INT || b.type != VAL_INT) {
    fprintf(stderr, "Type error: add expects two integers\n");
    exit(1);
  }
  Value result;
  result.type = VAL_INT;
  result.data.int_val = a.data.int_val + b.data.int_val;
  return result;
}

Value prim_subtract(Value a, Value b) {
  if (a.type != VAL_INT || b.type != VAL_INT) {
    fprintf(stderr, "Type error: subtract expects two integers\n");
    exit(1);
  }

  Value result;
  result.type = VAL_INT;
  result.data.int_val = a.data.int_val - b.data.int_val;
  return result;
}

Value prim_multiply(Value a, Value b) {
  if (a.type != VAL_INT || b.type != VAL_INT) {
    fprintf(stderr, "Type error: multiply expects two integers\n");
    exit(1);
  }

  Value result;
  result.type = VAL_INT;
  result.data.int_val = a.data.int_val * b.data.int_val;
  return result;
}

Value prim_equals(Value a, Value b) {
  Value result;
  result.type = VAL_BOOL;

  if (a.type != b.type) {
    result.data.bool_val = false;
    return result;
  }

  switch (a.type) {
  case VAL_UNIT:
    result.data.bool_val = true;
    break;

  case VAL_INT:
    result.data.bool_val = a.data.int_val == b.data.int_val;
    break;

  case VAL_BOOL:
    result.data.bool_val = a.data.bool_val == b.data.bool_val;
    break;

  default:
    result.data.bool_val = false;
  }

  return result;
}

Value prim_if(Value cond, Value then_val, Value else_val) {
  if (cond.type != VAL_BOOL) {
    fprintf(stderr, "Type error: if expects a boolean condition\n");
    exit(1);
  }
  return cond.data.bool_val ? then_val : else_val;
}

Value prim_succ(Value val) {
  if (val.type != VAL_INT) {
    fprintf(stderr, "Type error: Succ expects an integer argument\n");
    exit(1);
  }
  Value result;
  result.type = VAL_INT;
  result.data.int_val = ++val.data.int_val;
  return result;
}

// returns a newly constructed env. add
Env *init_standard_env() {
  Env *env = NULL;
  env = extend_env("add", make_primitive(PRIM_ADD), env);
  env = extend_env("subtract", make_primitive(PRIM_SUBTRACT), env);
  env = extend_env("multiply", make_primitive(PRIM_MULTIPLY), env);
  env = extend_env("equals", make_primitive(PRIM_EQUALS), env);
  env = extend_env("if", make_primitive(PRIM_IF), env);
  env = extend_env("succ", make_primitive(PRIM_SUCC), env);
  return env;
}
