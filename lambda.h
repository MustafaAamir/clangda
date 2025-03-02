#pragma once

#include "types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Type of expressions
typedef enum {
  EXP_UNIT, EXP_INT, EXP_BOOL,
  EXP_VAR, EXP_LAMBDA, EXP_APPLY,
  EXP_LET // Let binding (e.g., let x = e1 in e2)
} ExpType;

typedef enum {
  PRIM_ADD, PRIM_SUBTRACT, PRIM_MULTIPLY,
  PRIM_EQUALS, PRIM_IF, PRIM_SUCC
} PrimitiveOp;

// Forward declaration for Environment
typedef struct Env Env;

// Expression structure
typedef struct Exp {
  ExpType type;
  Type *inferred_type;
  union {
    unsigned int int_val;    // For EXP_INT
    bool bool_val;  // For EXP_BOOL
    char *var_name; // For EXP_VAR
    struct {        // For EXP_LAMBDA
      char *param;
      struct Exp *body;
    } lambda;
    struct { // For EXP_APPLY
      struct Exp *fn;
      struct Exp *arg;
    } apply;
    struct { // For EXP_LET
      char *var;
      struct Exp *e1;
      struct Exp *e2;
    } let;
  } data;
} Exp;

// Closure to represent functions with their environment
typedef struct {
  char *param;
  Exp *body;
  Env *env;
} Closure;

// Value representation for evaluation
typedef struct Value Value;
typedef struct Value {
  enum { VAL_UNIT, VAL_INT, VAL_BOOL, VAL_CLOSURE, VAL_PRIMITIVE } type;
  union {
    unsigned int int_val;
    bool bool_val;
    Closure closure;
    struct {
      struct Value *arg2;
      struct Value *arg1;
      struct Value *arg3;
      PrimitiveOp op;
      unsigned int num_args;
    } primitive;
  } data;
} Value;

Value make_primitive(PrimitiveOp op);
Value apply_primitive(Value *prim, Value *arg);

// Environment for variable bindings
struct Env {
  char *name;
  Value value;
  Env *next;
};

// Function declarations
Exp *make_int(unsigned int val);
Exp *make_bool(bool val);
Exp *make_var(const char *name);
Exp *make_lambda(const char *param, Exp *body);
Exp *make_apply(Exp *fn, Exp *arg);
Exp *make_let(const char *var, Exp *val, Exp *body);
Exp *make_unit();

Env *extend_env(const char *name, Value value, Env *env);
Value *lookup_env(const char *name, Env *env);
void free_env(Env *env);

Value eval(Exp *exp, Env *env);
void free_exp(Exp *exp);
void free_value(Value value);

// Utility functions
void print_exp(Exp *exp);
void string_of_value(Value value);
