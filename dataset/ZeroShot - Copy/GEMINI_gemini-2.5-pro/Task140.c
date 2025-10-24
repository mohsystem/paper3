#include <stdio.h>

/**
 * @brief Performs a simple arithmetic operation.
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
            break;
        case '-':
            *result = num1 - num2;
            break;
        case '*':
            *result = num1 * num2;
            break;
        case '/':
            if (num2 == 0) {
                return 1; // Error code for division by zero
            }
            *result = num1 / num2;
            break;
        default:
            return 2; // Error code for invalid operator
    }
    return 0; // Success
}

int main() {
    typedef struct {
        double num1;
        double num2;
        char op;
    } TestCase;

    TestCase test_cases[5] = {
        {10.0, 5.0, '+'},
        {20.5, 4.5, '-'},
        {7.0, 6.0, '*'},
        {15.0, 4.0, '/'},
        {10.0, 0.0, '/'}
    };

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < 5; i++) {
        double result;
        int status = calculate(test_cases[i].num1, test_cases[i].num2, test_cases[i].op, &result);

        printf("Test Case %d: %.2f %c %.2f -> ", i + 1, test_cases[i].num1, test_cases[i].op, test_cases[i].num2);

        switch (status) {
            case 0: // Success
                printf("%.2f\n", result);
                break;
            case 1: // Division by zero
                printf("Error: Division by zero is not allowed.\n");
                break;
            case 2: // Invalid operator
                printf("Error: Invalid operator. Only +, -, *, / are supported.\n");
                break;
        }
    }

    return 0;
}