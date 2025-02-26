#include "infer.h"
#include "lambda.h"
#include "parser.h"
#include "primitives.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper to evaluate an expression and check the result
void test_eval(const char *expr, Value expected) {
  printf("Testing: %s\n", expr);

  Env *env = init_standard_env();
  Exp *exp = parse(expr);

  Value result = eval(exp, env);

  // Check result type
  assert(result.type == expected.type);

  // Check value based on type
  switch (result.type) {
  case VAL_INT:
    assert(result.data.int_val == expected.data.int_val);
    printf("  Result: %d (expected %d)\n", result.data.int_val,
           expected.data.int_val);
    break;

  case VAL_BOOL:
    assert(result.data.bool_val == expected.data.bool_val);
    printf("  Result: %s (expected %s)\n",
           result.data.bool_val ? "true" : "false",
           expected.data.bool_val ? "true" : "false");
    break;
  case VAL_UNIT:
    printf("  Result: () (expected ())\n");
    break;

  case VAL_CLOSURE:
    printf("  Result: <function> (expected <function>)\n");
    break;

  case VAL_PRIMITIVE:
    printf("  Result: <primitive> (expected <primitive>)\n");
    break;
  }
}

// Helper to check that an expression has the expected type
void test_type(const char *expr, const char *expected_type) {
  printf("Testing type of: %s\n", expr);

  TypeEnv *env = init_standard_type_env();
  Exp *exp = parse(expr);

  Type *type = infer(exp, env);
  char *type_str = type_to_string(type);

  printf("  Type: %s (expected %s)\n", type_str, expected_type);
  assert(strcmp(type_str, expected_type) == 0);
}


// Test basic expressions
void test_basic_expressions() {
  printf("\n=== Testing Basic Expressions ===\n");

  // Integer literals
  Value expected_int = {.type = VAL_INT, .data = {.int_val = 42}};
  test_eval("42", expected_int);

  // Boolean literals
  Value expected_true = {.type = VAL_BOOL, .data = {.bool_val = true}};
  test_eval("true", expected_true);

  Value expected_false = {.type = VAL_BOOL, .data = {.bool_val = false}};
  test_eval("false", expected_false);

  // Identity function
  test_type("\\x.x", "'a -> 'a");

  // Identity function application
  test_eval("(\\x.x) 42", expected_int);

  // Constant function
  test_type("\\x.\\y.x", "'a -> 'b -> 'a");

  // Function application with multiple arguments
  //test_eval("((\\x.\\y.x) 42) 24", expected_int);
}

// Test arithmetic primitives
void test_arithmetic() {
  printf("\n=== Testing Arithmetic Primitives ===\n");

  // Addition
  Value expected_sum = {.type = VAL_INT, .data = {.int_val = 15}};
  test_eval("(add 10 5)", expected_sum);

  // Subtraction
  Value expected_diff = {.type = VAL_INT, .data = {.int_val = 5}};
  test_eval("subtract 10 5", expected_diff);

  // Multiplication
  Value expected_prod = {.type = VAL_INT, .data = {.int_val = 50}};
  test_eval("multiply 10 5", expected_prod);

  // Check types
  test_type("add", "unit -> unit -> unit");
  test_type("subtract", "unit -> unit -> unit");
  test_type("multiply", "unit -> unit -> unit");

  // Nested arithmetic
  Value expected_nested = {.type = VAL_INT, .data = {.int_val = 25}};
  test_eval("add (multiply 2 10) 5", expected_nested);

  // Error cases
}

// Test comparison primitives
void test_comparisons() {
  printf("\n=== Testing Comparison Primitives ===\n");

  // Equality - true cases
  Value expected_true = {.type = VAL_BOOL, .data = {.bool_val = true}};
  test_eval("equals 5 5", expected_true);
  test_eval("equals true true", expected_true);

  // Equality - false cases
  Value expected_false = {.type = VAL_BOOL, .data = {.bool_val = false}};
  test_eval("equals 5 10", expected_false);
  test_eval("equals true false", expected_false);

  // Different types should not be equal
  test_eval("equals 5 true", expected_false);

  // Check type
  test_type("equals", "'a -> 'a -> unit");
}

// Test if-then-else construct
void test_if_then_else() {
  printf("\n=== Testing If-Then-Else Construct ===\n");

  // Basic if-then-else with true condition
  Value expected_true_branch = {.type = VAL_INT, .data = {.int_val = 1}};
  test_eval("if true 1 2", expected_true_branch);

  // Basic if-then-else with false condition
  Value expected_false_branch = {.type = VAL_INT, .data = {.int_val = 2}};
  test_eval("if false 1 2", expected_false_branch);

  // Nested if-then-else
  test_eval("if (equals 5 5) (if true 1 2) 3", expected_true_branch);

  // If with computed condition
  test_eval("if (equals (add 2 3) 5) 1 2", expected_true_branch);

  // Check type
  test_type("if", "unit -> 'a -> 'a -> 'a");

  // Error cases

  // Both branches must have the same type
  test_type("if true 1 2", "unit");
  test_type("if true \\x.x \\y.y", "'a -> 'a");
}

// Test let bindings
void test_let_bindings() {
  printf("\n=== Testing Let Bindings ===\n");

  // Simple let binding
  Value expected_int = {.type = VAL_INT, .data = {.int_val = 42}};
  test_eval("let x = 42 in x", expected_int);

  // Let binding with function
  test_eval("let id = \\x.x in id 42", expected_int);

  // Nested let bindings
  test_eval("let x = 40 in let y = 2 in add x y", expected_int);

  // Let with function application
  test_eval("let double = \\x.add x x in double 21", expected_int);

  // Let polymorphism
  test_type("let id = \\x.x in id", "'a -> 'a");
  test_type("let id = \\x.x in id 42", "unit");
  test_type("let id = \\x.x in id true", "unit");

  // Let with recursive function
  Value expected_factorial = {.type = VAL_INT, .data = {.int_val = 120}};
  test_eval("let fact = \\n.if (equals n 0) 1 (multiply n (fact (subtract n "
            "1))) in fact 5",
            expected_factorial);

  // Let with multiple recursion
  Value expected_fib = {.type = VAL_INT, .data = {.int_val = 5}};
  test_eval("let fib = \\n.if (equals n 0) 0 (if (equals n 1) 1 (add (fib "
            "(subtract n 1)) (fib (subtract n 2)))) in fib 5",
            expected_fib);
}

// Test higher-order functions
void test_higher_order() {
  printf("\n=== Testing Higher-Order Functions ===\n");

  // Function composition
  Value expected_int = {.type = VAL_INT, .data = {.int_val = 42}};
  test_eval("let compose = \\f.\\g.\\x.f (g x) in let double = \\x.add x x in "
            "let inc = \\x.add x 1 in ((compose double inc) 20)",
            expected_int);

  // Function passed as argument
  test_eval("let apply = \\f.\\x.f x in let inc = \\x.add x 1 in apply inc 41",
            expected_int);

  // Function returned as result
  test_eval("let make_adder = \\n.\\x.add n x in let add10 = make_adder 10 in "
            "add10 32",
            expected_int);

  // Map function
  test_type("\\f.\\x.f x", "('a -> 'b) -> 'a -> 'b");

  // Y combinator
  test_type("\\f.(\\x.f (x x)) (\\x.f (x x))", "('a -> 'a) -> 'a");
}

// Run all tests
int main() {
  printf("Running Lambda Calculus Interpreter Tests\n");
  printf("=========================================\n");

  // Basic expressions
  test_basic_expressions();

  // Arithmetic primitives
  test_arithmetic();

  // Comparison primitives
  test_comparisons();

  // If-then-else construct
  test_if_then_else();

  // Let bindings
  test_let_bindings();

  // Higher-order functions
  test_higher_order();

  printf("\nAll tests passed!\n");
  return 0;
}
