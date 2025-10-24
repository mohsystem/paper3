#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Performs a basic arithmetic operation.
 * 
 * @param num1 The first operand.
 * @param num2 The second operand.
 * @param op The operator character (+, -, *, /).
 * @param result Pointer to a double to store the result.
 * @return true if the operation is successful, false otherwise.
 */
bool calculate(double num1, double num2, char op, double* result) {
    if (result == NULL) {
        return false; // Cannot store the result
    }
    
    switch (op) {
        case '+':
            *result = num1 + num2;
            return true;
        case '-':
            *result = num1 - num2;
            return true;
        case '*':
            *result = num1 * num2;
            return true;
        case '/':
            if (num2 == 0.0) {
                // Division by zero error
                return false;
            }
            *result = num1 / num2;
            return true;
        default:
            // Invalid operator
            return false;
    }
}

int main() {
    // Test cases
    double nums1[] = {10.0, 15.0, 7.5, 12.0, 10.0};
    double nums2[] = {5.0, 3.0, 2.5, 0.0, 3.0};
    char ops[] = {'+', '*', '-', '/', '%'};

    for (int i = 0; i < 5; i++) {
        double num1 = nums1[i];
        double num2 = nums2[i];
        char op = ops[i];
        double result;

        printf("Test Case %d: %.2f %c %.2f\n", i + 1, num1, op, num2);

        if (calculate(num1, num2, op, &result)) {
            printf("Result: %.2f\n\n", result);
        } else {
            // Distinguish the error type for a better message
            if (op == '/' && num2 == 0.0) {
                fprintf(stderr, "Error: Division by zero is not allowed.\n\n");
            } else if (op != '+' && op != '-' && op != '*' && op != '/') {
                fprintf(stderr, "Error: Invalid operator. Only +, -, *, / are allowed.\n\n");
            } else {
                fprintf(stderr, "An unknown error occurred.\n\n");
            }
        }
    }

    return 0;
}