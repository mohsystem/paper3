
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum {
    SUCCESS = 0,
    ERROR_INVALID_OPERATOR = 1,
    ERROR_DIVISION_BY_ZERO = 2
} CalculationStatus;

CalculationStatus calculate(double num1, double num2, const char* operator, double* result) {
    // Validate operator input (whitelist approach)
    if (operator == NULL || strlen(operator) != 1) {
        return ERROR_INVALID_OPERATOR;
    }
    
    char op = operator[0];
    
    switch (op) {
        case '+':
            *result = num1 + num2;
            return SUCCESS;
        case '-':
            *result = num1 - num2;
            return SUCCESS;
        case '*':
            *result = num1 * num2;
            return SUCCESS;
        case '/':
            // Secure division with zero-check
            if (num2 == 0 || fabs(num2) < 1e-10) {
                return ERROR_DIVISION_BY_ZERO;
            }
            *result = num1 / num2;
            return SUCCESS;
        default:
            return ERROR_INVALID_OPERATOR;
    }
}

const char* get_error_message(CalculationStatus status) {
    switch (status) {
        case ERROR_INVALID_OPERATOR:
            return "Invalid operator. Use +, -, *, or /";
        case ERROR_DIVISION_BY_ZERO:
            return "Division by zero is not allowed";
        default:
            return "Unknown error";
    }
}

int main() {
    double result;
    CalculationStatus status;
    
    // Test case 1: Addition
    status = calculate(10, 5, "+", &result);
    if (status == SUCCESS) {
        printf("Test 1: 10 + 5 = %.2f\\n", result);
    } else {
        printf("Error: %s\\n", get_error_message(status));
    }
    
    // Test case 2: Subtraction
    status = calculate(20, 8, "-", &result);
    if (status == SUCCESS) {
        printf("Test 2: 20 - 8 = %.2f\\n", result);
    } else {
        printf("Error: %s\\n", get_error_message(status));
    }
    
    // Test case 3: Multiplication
    status = calculate(7, 6, "*", &result);
    if (status == SUCCESS) {
        printf("Test 3: 7 * 6 = %.2f\\n", result);
    } else {
        printf("Error: %s\\n", get_error_message(status));
    }
    
    // Test case 4: Division
    status = calculate(15, 3, "/", &result);
    if (status == SUCCESS) {
        printf("Test 4: 15 / 3 = %.2f\\n", result);
    } else {
        printf("Error: %s\\n", get_error_message(status));
    }
    
    // Test case 5: Division by zero (error handling)
    status = calculate(10, 0, "/", &result);
    if (status == SUCCESS) {
        printf("Test 5: 10 / 0 = %.2f\\n", result);
    } else {
        printf("Test 5 Error: %s\\n", get_error_message(status));
    }
    
    return 0;
}
