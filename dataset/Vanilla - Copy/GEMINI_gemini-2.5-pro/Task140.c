#include <stdio.h>
#include <math.h> // Required for NAN and isnan

/**
 * @brief Performs a basic arithmetic operation.
 *
 * @param num1 The first number.
 * @param num2 The second number.
 * @param op The operator (+, -, *, /).
 * @return The result of the operation, or NAN if the operation is invalid.
 */
double calculate(double num1, double num2, char op) {
    switch (op) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            if (num2 != 0) {
                return num1 / num2;
            } else {
                fprintf(stderr, "Error: Division by zero is not allowed.\n");
                return NAN;
            }
        default:
            fprintf(stderr, "Error: Invalid operator '%c'.\n", op);
            return NAN;
    }
}

int main() {
    // Test Case 1: Addition
    double num1_1 = 10, num2_1 = 5;
    char op1 = '+';
    printf("Test Case 1: %.1f %c %.1f = %.1f\n", num1_1, op1, num2_1, calculate(num1_1, num2_1, op1));

    // Test Case 2: Subtraction
    double num1_2 = 20.5, num2_2 = 10.5;
    char op2 = '-';
    printf("Test Case 2: %.1f %c %.1f = %.1f\n", num1_2, op2, num2_2, calculate(num1_2, num2_2, op2));

    // Test Case 3: Multiplication
    double num1_3 = 7, num2_3 = 8;
    char op3 = '*';
    printf("Test Case 3: %.1f %c %.1f = %.1f\n", num1_3, op3, num2_3, calculate(num1_3, num2_3, op3));

    // Test Case 4: Division
    double num1_4 = 100, num2_4 = 4;
    char op4 = '/';
    printf("Test Case 4: %.1f %c %.1f = %.1f\n", num1_4, op4, num2_4, calculate(num1_4, num2_4, op4));

    // Test Case 5: Division by zero
    double num1_5 = 15, num2_5 = 0;
    char op5 = '/';
    printf("Test Case 5: %.1f %c %.1f = ", num1_5, op5, num2_5);
    double result5 = calculate(num1_5, num2_5, op5);
    if (isnan(result5)) {
        printf("Undefined\n");
    } else {
        printf("%.1f\n", result5);
    }

    return 0;
}