#include "infer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Main type inference function
Type *infer(Exp *exp, TypeEnv *env) {
  switch (exp->type) {
  case EXP_UNIT:
    return new_MT_type(TYPE_UNIT);
  case EXP_INT: {
    return new_MT_type(TYPE_INT);
  }
  case EXP_BOOL: {
    return new_MT_type(TYPE_BOOL);
  }

  case EXP_VAR: {
    // Look up the variable in the environment
    PolyType *polytype = lookup_type_env(exp->data.var_name, env);
    if (polytype == NULL) {
      fprintf(stderr, "Type error: unbound variable %s\n", exp->data.var_name);
      exit(1);
    }

    // Instantiate the polymorphic type
    Type *t = instantiate(polytype);
    exp->inferred_type = t;
    return t;
  }

  case EXP_LAMBDA: {
    // Create a fresh type variable for the parameter
    Type *param_type = new_typevar();

    // Extend the environment with the parameter
    PolyType *param_polytype = dont_generalize(param_type);
    TypeEnv *new_env =
        extend_type_env(exp->data.lambda.param, param_polytype, env);

    // Infer the type of the body
    Type *body_type = infer(exp->data.lambda.body, new_env);

    // The type of the lambda is param_type -> body_type
    Type *fn_type = type_function(param_type, body_type);
    exp->inferred_type = fn_type;

    // Free the extended environment
    free(new_env->name);
    free_polytype(new_env->type);
    free(new_env);

    return fn_type;
  }

  case EXP_APPLY: {
    // Infer the type of the function
    Type *fn_type = infer(exp->data.apply.fn, env);

    // Infer the type of the argument
    Type *arg_type = infer(exp->data.apply.arg, env);

    // Create a fresh type variable for the result
    Type *result_type = new_typevar();

    // The function type must be arg_type -> result_type
    Type *expected_fn_type = type_function(arg_type, result_type);

    // Unify the actual function type with the expected function type
    unify(fn_type, expected_fn_type);

    exp->inferred_type = result_type;
    return result_type;
  }

  case EXP_LET: {
    // Create a temporary environment for recursive definitions
    TypeEnv *temp_env = env;
    Type *var_type = new_typevar();
    PolyType *var_polytype = dont_generalize(var_type);
    temp_env = extend_type_env(exp->data.let.var, var_polytype, temp_env);

    // Enter a new type level for polymorphism
    enter_level();

    // Infer the type of the value in the extended environment
    Type *val_type = infer(exp->data.let.e1, temp_env);

    // Unify the variable type with the value type
    unify(var_type, val_type);

    // Exit the type level
    exit_level();

    // Generalize the type
    PolyType *val_polytype = generalize(val_type);

    // Extend the environment with the generalized type
    TypeEnv *new_env = extend_type_env(exp->data.let.var, val_polytype, env);

    // Infer the type of the body in the extended environment
    Type *body_type = infer(exp->data.let.e2, new_env);

    exp->inferred_type = body_type;

    // Free the extended environment
    free(new_env->name);
    free_polytype(new_env->type);
    free(new_env);

    return body_type;
  }
  }

  // Should never reach here
  fprintf(stderr, "Type error: unknown expression type\n");
  exit(1);
}

// Initialize the standard environment with primitive types
TypeEnv *init_standard_type_env() {
  TypeEnv *env = NULL;

  // Example: add : int -> int -> int
  Type *int_type = new_MT_type(TYPE_INT);
  Type *add_type = type_function(int_type, type_function(int_type, int_type));
  PolyType *add_polytype = dont_generalize(add_type);
  env = extend_type_env("add", add_polytype, env);

  // Example: subtract : int -> int -> int
  Type *subtract_type =
      type_function(int_type, type_function(int_type, int_type));
  PolyType *subtract_polytype = dont_generalize(subtract_type);
  env = extend_type_env("subtract", subtract_polytype, env);

  // Example: multiply : int -> int -> int
  Type *multiply_type =
      type_function(int_type, type_function(int_type, int_type));
  PolyType *multiply_polytype = dont_generalize(multiply_type);
  env = extend_type_env("multiply", multiply_polytype, env);

  // Example: equals : 'a -> 'a -> bool
  Type *a_type = new_typevar();
  Type *bool_type = new_MT_type(TYPE_BOOL);
  Type *equals_type = type_function(a_type, type_function(a_type, bool_type));
  PolyType *equals_polytype = generalize(equals_type);
  env = extend_type_env("equals", equals_polytype, env);

  // Example: if : bool -> 'a -> 'a -> 'a
  Type *b_type = new_typevar();
  Type *if_type = type_function(
      bool_type, type_function(b_type, type_function(b_type, b_type)));
  PolyType *if_polytype = generalize(if_type);
  env = extend_type_env("if", if_polytype, env);

// Example: multiply : int -> int -> int
  Type *succ_type =
      type_function(int_type, int_type);
  PolyType *succ_polytype = dont_generalize(succ_type);
  env = extend_type_env("succ", succ_polytype, env);


  return env;
}
