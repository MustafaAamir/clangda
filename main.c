#include "infer.h"
#include "lambda.h"
#include "parser.h"
#include "primitives.h"
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 1024

jmp_buf error_jmp_buf;
char error_message[256];

void throw_error(const char *message) {
  strncpy(error_message, message, 255);
  error_message[255] = '\0';
  longjmp(error_jmp_buf, 1);
}
#define MAX_LINE_LENGTH 1024
bool process_file_line_by_line(const char *filename, Env *runtime_env, TypeEnv *type_env) {
  FILE *file = NULL;
  char line[MAX_LINE_LENGTH];
  int line_count = 0;

  // Open the file
  file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
    return false;
  }

  // Read and process the file line by line
  while (fgets(line, sizeof(line), file) != NULL) {
    line_count++;

    // Remove trailing newline if present
    size_t line_len = strlen(line);
    if (line_len > 0 && line[line_len - 1] == '\n') {
      line[line_len - 1] = '\0';
      line_len--;
    }

    // Skip empty lines
    if (line_len == 0) {
      continue;
    }

    // Process the line
    printf("Line %d: %s\n", line_count, line);

    Exp *exp = NULL;

    if (setjmp(error_jmp_buf) == 0) {
      exp = parse(line);
      printf("Expression: ");
      print_exp(exp);
      printf("\n");

      Type *type = infer(exp, type_env);
      char *type_str = type_to_string(type);
      printf("Type: %s\n", type_str);

      Value result = eval(exp, runtime_env);
      printf("Value: ");
      string_of_value(result);
      printf("\n\n");

      // Free the expression when done
    } else {
      fprintf(stderr, "Error on line %d: %s\n\n", line_count, error_message);
    }
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
  Exp *exp = make_apply(make_apply(make_lambda("x", make_lambda("y", make_var("x"))), make_int(1)), make_int(2));
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
  // debug(runtime_env, type_env);
  printf("Lambda Calculus Interpreter with Hindley-Milner Type Inference\n");
  printf("Type 'exit' to quit\n\n");
  if (argc == 2) {
    const char *filename = argv[1];
    if (!process_file_line_by_line(filename, runtime_env, type_env)) {
      return EXIT_FAILURE;
    }
    return EXIT_FAILURE;
  } else {
    while (1) {
      printf("> ");
      if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
        break;
      }
      input[strcspn(input, "\n")] = '\0';
      if (strcmp(input, "exit") == 0) {
        break;
      }
      if (strlen(input) == 0) {
        continue;
      }
      current_level = 0;
      current_typevar = 0;
      Exp *exp = NULL;
      if (setjmp(error_jmp_buf) == 0) {
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
}
