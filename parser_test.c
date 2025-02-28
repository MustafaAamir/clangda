#include "lambda.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Testing parser with nested lambdas and application...\n");
    
    // Test 1: Basic lambda application
    printf("\nTest 1: (\\x.x) 1\n");
    Exp *exp1 = parse("(\\x.x) 1");
    printf("Parsed expression: ");
    print_exp(exp1);
    printf("\n");
    free_exp(exp1);
    
    // Test 2: Nested lambda
    printf("\nTest 2: \\x.\\y.x\n");
    Exp *exp2 = parse("\\x.\\y.x");
    printf("Parsed expression: ");
    print_exp(exp2);
    printf("\n");
    free_exp(exp2);
    
    // Test 3: The problematic expression
    printf("\nTest 3: (\\x.\\y.x) 1 2\n");
    Exp *exp3 = NULL;
    try {
        exp3 = parse("(\\x.\\y.x) 1 2");
        printf("Parsed expression: ");
        print_exp(exp3);
        printf("\n");
        free_exp(exp3);
    } catch (...) {
        printf("Parser error occurred\n");
    }
    
    // Test 4: Manual construction of the expression
    printf("\nTest 4: Manually building (\\x.\\y.x) 1 2\n");
    Exp *lambda_body = make_var("x");
    Exp *lambda = make_lambda("y", lambda_body);
    Exp *outer_lambda = make_lambda("x", lambda);
    Exp *app1 = make_apply(outer_lambda, make_int(1));
    Exp *app2 = make_apply(app1, make_int(2));
    
    printf("Manually constructed expression: ");
    print_exp(app2);
    printf("\n");
    free_exp(app2);
    
    return 0;
}