#include "infer.h"
#include "lambda.h"
#include "parser.h"
#include "primitives.h"
#include <errno.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 1024

#define MAX_LINE_LENGTH 1024
bool process_file_line_by_line(const char *filename, Env *runtime_env, TypeEnv *type_env) {
  FILE *file = NULL;
  char line[MAX_LINE_LENGTH];
  int line_count = 0;
  file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
    return false;
  }
  while (fgets(line, sizeof(line), file) != NULL) {
    line_count++;
    size_t line_len = strlen(line);
    if (line_len > 0 && line[line_len - 1] == '\n') {
      line[line_len - 1] = '\0';
      line_len--;
    }
    if (line_len == 0) {
      continue;
    }

    // Process the line
    printf("Line %d: %s\n", line_count, line);

    Exp *exp = NULL;

    exp = parse(line);
    printf("Expression: ");
    print_exp(exp);
    printf("\n");

    // Type *type = infer(exp, type_env);
    // char *type_str = type_to_string(type);
    // printf("Type: %s\n", type_str);

    Value result = eval(exp, runtime_env);
    printf("Value: ");
    string_of_value(result);
    printf("\n\n");

    // Free the expression when done
  }

  // Check if we stopped because of an error
  if (ferror(file)) {
    fprintf(stderr, "Error reading file: %s\n", strerror(errno));
    fclose(file);
    return false;
  }

  // Cleanup
  fclose(file);
  return true;
}
void debug(Env *runtime_env, TypeEnv *type_env) {
  Exp *exp = make_apply(make_apply(make_lambda("x", make_lambda("y", make_var("y"))), make_int(1)), make_int(2));
  Type *type = infer(exp, type_env);
  char *type_str = type_to_string(type);
  Value result = eval(exp, runtime_env);
  printf("Type: %s\n", type_str);
  printf("Value: ");
  string_of_value(result);
  printf("\n\n");
}

int main(int argc, char *argv[]) {
  char input[INPUT_BUFFER_SIZE];
  Env *runtime_env = init_standard_env();
  TypeEnv *type_env = init_standard_type_env();
  debug(runtime_env, type_env);
  printf("Lambda Calculus Interpreter with Hindley-Milner Type Inference\n");
  printf("Type 'exit' to quit\n\n");
  if (argc == 2) {
    const char *filename = argv[1];
    if (!process_file_line_by_line(filename, runtime_env, type_env)) {
      return EXIT_FAILURE;
    }
    return EXIT_FAILURE;
  } else {

    char *input = NULL;
    while (1) {
      input = readline("> ");
      if (!input) {
        break; // Handle EOF (Ctrl+D)
      }
      if (strlen(input) == 0) {
        free(input);
        continue;
      }

      add_history(input); // Add to history

      current_level = 0;
      current_typevar = 0;
      Exp *exp = NULL;

      exp = parse(input);
      print_exp(exp);
      printf("\n");
      Type *type = infer(exp, type_env);
      char *type_str = type_to_string(type);
      Value result = eval(exp, runtime_env);
      printf("Type: %s\n", type_str);
      printf("Value: ");
      string_of_value(result);
      printf("\n\n");
    }
    free(input);
  }
  // Free environments
  free_env(runtime_env);
  free_type_env(type_env);
  return 0;
}
