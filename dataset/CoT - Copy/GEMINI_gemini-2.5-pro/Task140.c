#include <stdio.h>

/**
 * Performs a basic arithmetic operation on two numbers.
 *
 * @param num1 The first number.
 * @param num2 The second number.
 * @param op The operator character (+, -, *, /).
 * @param result A pointer to a double where the result will be stored.
 * @return 0 on success, 1 for division by zero, 2 for invalid operator.
 */
int calculate(double num1, double num2, char op, double* result) {
    switch (op) {
        case '+':
            *result = num1 + num2;
            return 0; // Success
        case '-':
            *result = num1 - num2;
            return 0; // Success
        case '*':
            *result = num1 * num2;
            return 0; // Success
        case '/':
            if (num2 == 0) {
                return 1; // Error: Division by zero
            }
            *result = num1 / num2;
            return 0; // Success
        default:
            return 2; // Error: Invalid operator
    }
}

void run_test(int test_num, double n1, double n2, char op, const char* op_str) {
    double res;
    int status = calculate(n1, n2, op, &res);
    printf("Test %d: %.1f %s %.1f = ", test_num, n1, op_str, n2);
    if (status == 0) {
        printf("%.2f\n", res);
    } else if (status == 1) {
        printf("Error: Division by zero is not allowed.\n");
    } else {
        printf("Error: Invalid operator.\n");
    }
}

int main() {
    // Test Case 1: Addition
    run_test(1, 10.5, 2.5, '+', "+");
    
    // Test Case 2: Subtraction
    run_test(2, 20.0, 5.5, '-', "-");

    // Test Case 3: Multiplication
    run_test(3, 5.0, 4.0, '*', "*");

    // Test Case 4: Division by Zero (Error)
    run_test(4, 10.0, 0.0, '/', "/");

    // Test Case 5: Invalid Operator (Error)
    run_test(5, 10.0, 5.0, '%', "%");

    return 0;
}