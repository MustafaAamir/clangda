#include "infer.h"
#include "lambda.h"
#include "parser.h"
#include "primitives.h"
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 1024

// Global error handling
jmp_buf error_jmp_buf;
char error_message[256];

void throw_error(const char *message) {
  strncpy(error_message, message, 255);
  error_message[255] = '\0';
  longjmp(error_jmp_buf, 1);
}

int main() {
  char input[INPUT_BUFFER_SIZE];

  // Initialize environments
  Env *runtime_env = init_standard_env();
  TypeEnv *type_env = init_standard_type_env();
  Exp *exp = make_apply(make_apply(make_var("add"), make_int(1)), make_int(2));
  Type *type = infer(exp, type_env);
  char *type_str = type_to_string(type);

  // Evaluate
  Value result = eval(exp, runtime_env);

  // Print results
  printf("Type: %s\n", type_str);
  printf("Value: ");
  string_of_value(result);
  printf("\n\n");

  printf("Lambda Calculus Interpreter with Hindley-Milner Type Inference\n");
  printf("Type 'exit' to quit\n\n");

  while (1) {
    printf("> ");
    if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
      break;
    }

    // Remove newline
    input[strcspn(input, "\n")] = '\0';

    // Check for exit
    if (strcmp(input, "exit") == 0) {
      break;
    }

    // Skip empty lines
    if (strlen(input) == 0) {
      continue;
    }

    // Reset type inference state
    current_level = 0;
    current_typevar = 0;

    // Parse and evaluate with error handling
    Exp *exp = NULL;

    if (setjmp(error_jmp_buf) == 0) {
      // Normal execution path

      // Parse the input
      exp = parse(input);

      // Type inference
      Type *type = infer(exp, type_env);
      char *type_str = type_to_string(type);

      // Evaluate
      Value result = eval(exp, runtime_env);

      // Print results
      printf("Type: %s\n", type_str);
      printf("Value: ");
      string_of_value(result);
      printf("\n\n");

    } else {
      throw_error(error_message);
      if (exp != NULL) {
        free_exp(exp);
      }
    }
  }

  // Free environments
  free_env(runtime_env);
  free_type_env(type_env);

  return 0;
}
