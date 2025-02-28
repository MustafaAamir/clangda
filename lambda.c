#include "lambda.h"
#include "primitives.h"

static Exp *safe_malloc() {
  void *p = malloc(sizeof(Exp));
  if (p == NULL) {
    fprintf(stderr, "Fatal: failed to allocate %zu bytes for expression.\n", sizeof(Exp));
    exit(1);
  }
  return p;
}

Exp *make_int(unsigned int val) {
  Exp *exp = safe_malloc();
  exp->type = EXP_INT;
  exp->data.int_val = val;
  exp->inferred_type = NULL;
  return exp;
}

Exp *make_bool(bool val) {
  Exp *exp = safe_malloc();
  exp->type = EXP_BOOL;
  exp->data.bool_val = val;
  exp->inferred_type = NULL;
  return exp;
}

Exp *make_var(const char *name) {
  Exp *exp = safe_malloc();
  exp->type = EXP_VAR;
  exp->data.var_name = strdup(name);
  exp->inferred_type = NULL;
  return exp;
}

Exp *make_lambda(const char *param, Exp *body) {
  Exp *exp = (Exp *)safe_malloc();
  exp->type = EXP_LAMBDA;
  exp->data.lambda.param = strdup(param);
  exp->data.lambda.body = body;
  exp->inferred_type = NULL;
  return exp;
}

Exp *make_apply(Exp *fn, Exp *arg) {
  Exp *exp = (Exp *)safe_malloc();
  exp->type = EXP_APPLY;
  exp->data.apply.fn = fn;
  exp->data.apply.arg = arg;
  exp->inferred_type = NULL;
  return exp;
}

Exp *make_let(const char *var, Exp *e1, Exp *e2) {
  Exp *exp = safe_malloc();
  exp->type = EXP_LET;
  exp->data.let.var = strdup(var);
  exp->data.let.e1 = e1;
  exp->data.let.e2 = e2;
  exp->inferred_type = NULL;
  return exp;
}

Exp *make_unit() {
  Exp *exp = safe_malloc();
  exp->type = EXP_UNIT;
  exp->inferred_type = NULL;
  return exp;
}

// Environment operations
Env *extend_env(const char *name, Value value, Env *env) {
  Env *new_env = malloc(sizeof(Env));
  new_env->name = strdup(name);
  new_env->value = value;
  new_env->next = env;
  return new_env;
}

Value make_primitive(PrimitiveOp op) {
  Value v;
  v.type = VAL_PRIMITIVE;
  v.data.primitive.op = op;
  v.data.primitive.num_args = 0;
  return v;
}

// Apply an argument to a primitive
Value apply_primitive(Value prim, Value arg) {
  if (prim.type != VAL_PRIMITIVE) {
    fprintf(stderr, "Cannot apply to non-primitive");
    exit(1);
  }

  if (prim.data.primitive.num_args < 1) {
    prim.data.primitive.arg1 = &arg;
    ++(prim.data.primitive.num_args);
  } else if (prim.data.primitive.num_args < 2) {
    prim.data.primitive.arg2 = &arg;
    ++(prim.data.primitive.num_args);
  } else if (prim.data.primitive.num_args < 3) {
    prim.data.primitive.arg3 = &arg;
    ++(prim.data.primitive.num_args);
  }
  switch (prim.data.primitive.op) {
  case PRIM_SUCC:
    if (prim.data.primitive.num_args == 1)
      return prim_succ(*prim.data.primitive.arg1);
    break;
  case PRIM_ADD:
  case PRIM_SUBTRACT:
  case PRIM_MULTIPLY:
  case PRIM_EQUALS:
    if (prim.data.primitive.num_args == 2) {
      switch (prim.data.primitive.op) {
      case PRIM_ADD:
        return prim_add(*prim.data.primitive.arg1, *prim.data.primitive.arg2);
      case PRIM_SUBTRACT:
        return prim_subtract(*prim.data.primitive.arg1,
                             *prim.data.primitive.arg2);
      case PRIM_MULTIPLY:
        return prim_multiply(*prim.data.primitive.arg1,
                             *prim.data.primitive.arg2);
      case PRIM_EQUALS:
        return prim_equals(*prim.data.primitive.arg1,
                           *prim.data.primitive.arg2);
      case PRIM_IF:
      default:
        break;
      }
    }
    break;

  case PRIM_IF:
    if (prim.data.primitive.num_args == 3) {
      // If-then-else, we have all three arguments
      return prim_if(*prim.data.primitive.arg1, *prim.data.primitive.arg2,
                     *prim.data.primitive.arg3);
    }
    break;
  }

  // Not enough arguments yet, return the partially applied primitive
  return prim;
}

Value *lookup_env(const char *name, Env *env) {
  while (env != NULL) {
	printf("c");
    if (strcmp(env->name, name) == 0) {
      return &env->value;
    }
    env = env->next;
  }
  return NULL;
}

// Memory management
void free_exp(Exp *exp) {
  if (exp == NULL)
    return;

  switch (exp->type) {
  case EXP_VAR:
    free(exp->data.var_name);
    break;
  case EXP_LAMBDA:
    free(exp->data.lambda.param);
    free_exp(exp->data.lambda.body);
    break;
  case EXP_APPLY:
    free_exp(exp->data.apply.fn);
    free_exp(exp->data.apply.arg);
    break;
  case EXP_LET:
    free(exp->data.let.var);
    free_exp(exp->data.let.e1);
    free_exp(exp->data.let.e2);
    break;
  case EXP_INT:
  case EXP_BOOL:
  case EXP_UNIT:
    break;
  }
  if (exp->inferred_type != NULL) {
    free_type(exp->inferred_type);
  }
  free(exp);
}

void free_env(Env *env) {
  while (env != NULL) {
    Env *next = env->next;
    free(env->name);
    if (env->value.type == VAL_CLOSURE) {
      free(env->value.data.closure.param);
      // Do not free body and env to avoid circular freeing
    }
    free(env);
    env = next;
  }
}

void free_value(Value value) {
  if (value.type == VAL_CLOSURE) {
    free(value.data.closure.param);
    // We do not free the body and environment here to avoid double-freeing
  }
}

// The evaluator
Value eval(Exp *exp, Env *env) {
  Value result;
  Value *lookup_result;
  Value arg_val;

printf("a\n");

  switch (exp->type) {
  case EXP_UNIT:
    result.type = VAL_UNIT;
    return result;
  case EXP_INT:
    result.type = VAL_INT;
    result.data.int_val = exp->data.int_val;
    return result;

  case EXP_BOOL:
    result.type = VAL_BOOL;
    result.data.bool_val = exp->data.bool_val;
    return result;

  case EXP_VAR:
    lookup_result = lookup_env(exp->data.var_name, env);
    if (lookup_result == NULL) {
      fprintf(stderr, "Unbound variable: %s\n", exp->data.var_name);
      exit(1);
    }
    return *lookup_result;

  case EXP_LAMBDA:
    result.type = VAL_CLOSURE;
    result.data.closure.param = strdup(exp->data.lambda.param);
    result.data.closure.body = exp->data.lambda.body;
    result.data.closure.env = env;
    return result;

  case EXP_APPLY: {
    // Evaluate the function expression
    Value fn_val = eval(exp->data.apply.fn, env);

    if (fn_val.type != VAL_CLOSURE && fn_val.type != VAL_PRIMITIVE) {
      fprintf(stderr, "Cannot apply a non-function value\n");
      exit(1);
    }

    // Evaluate the argument expression
    arg_val = eval(exp->data.apply.arg, env);
    if (fn_val.type == VAL_CLOSURE) {
      // Create an extended environment for the function application
      Env *new_env = extend_env(fn_val.data.closure.param, arg_val,
                                fn_val.data.closure.env);

      // Evaluate the function body in the extended environment
      result = eval(fn_val.data.closure.body, new_env);

      // Free the environment we created (but not the original environments)
      free(new_env->name);
      free(new_env);

      return result;
    } else if (fn_val.type == VAL_PRIMITIVE) {
      return apply_primitive(fn_val, arg_val);
    }
  } break;
  case EXP_LET: {
    // Evaluate the value

    // Create extended environment
    Env *let_env =
        extend_env(exp->data.let.var, (Value){.type = VAL_UNIT}, env);

    Value val = eval(exp->data.let.e1, let_env);
    let_env->value = val;
    // Evaluate the body in the extended environment
    result = eval(exp->data.let.e2, let_env);

    // Free the environment we created
    free(let_env->name);
    free(let_env);

    return result;
  }
  }

  // Should never reach here
  fprintf(stderr, "Unknown expression type in eval\n");
  exit(1);
}

void print_exp(Exp *exp) {
  switch (exp->type) {
  case EXP_UNIT:
    printf("()");
    break;
  case EXP_INT:
    printf("%d", exp->data.int_val);
    break;
  case EXP_BOOL:
    printf("%s", exp->data.bool_val ? "true" : "false");
    break;
  case EXP_VAR:
    printf("%s", exp->data.var_name);
    break;
  case EXP_LAMBDA:
    printf("(lambda %s. ", exp->data.lambda.param);
    print_exp(exp->data.lambda.body);
    printf(")");
    break;
  case EXP_APPLY:
    printf("(");
    print_exp(exp->data.apply.fn);
    printf(" ");
    print_exp(exp->data.apply.arg);
    printf(")");
    break;
  case EXP_LET:
    printf("(let %s = ", exp->data.let.var);
    print_exp(exp->data.let.e1);
    printf(" in ");
    print_exp(exp->data.let.e2);
    printf(")");
    break;
  }
}

void string_of_value(Value value) {
  switch (value.type) {
  case VAL_UNIT:
    printf("()");
    break;
  case VAL_INT:
    printf("%d", value.data.int_val);
    break;
  case VAL_BOOL:
    printf("%s", value.data.bool_val ? "true" : "false");
    break;
  case VAL_CLOSURE:
    printf("<lambda>");
    break;
  case VAL_PRIMITIVE:
    printf("<primitive>");
    break;
  }
}
