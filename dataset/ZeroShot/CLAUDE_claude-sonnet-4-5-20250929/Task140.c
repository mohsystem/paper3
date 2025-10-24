
#include <stdio.h>
#include <stdbool.h>

bool calculate(double num1, double num2, char op, double* result) {
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
            if (num2 == 0) {
                printf("Error: Division by zero\\n");
                return false;
            }
            *result = num1 / num2;
            return true;
        default:
            printf("Error: Invalid operator\\n");
            return false;
    }
}

int main() {
    double result;
    
    // Test case 1: Addition
    if (calculate(10, 5, '+', &result)) {
        printf("Test 1 (10 + 5): %.2f\\n", result);
    }
    
    // Test case 2: Subtraction
    if (calculate(10, 5, '-', &result)) {
        printf("Test 2 (10 - 5): %.2f\\n", result);
    }
    
    // Test case 3: Multiplication
    if (calculate(10, 5, '*', &result)) {
        printf("Test 3 (10 * 5): %.2f\\n", result);
    }
    
    // Test case 4: Division
    if (calculate(10, 5, '/', &result)) {
        printf("Test 4 (10 / 5): %.2f\\n", result);
    }
    
    // Test case 5: Division by zero
    printf("Test 5 (10 / 0): ");
    calculate(10, 0, '/', &result);
    
    return 0;
}
