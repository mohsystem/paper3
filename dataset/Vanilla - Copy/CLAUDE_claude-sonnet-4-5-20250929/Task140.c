
#include <stdio.h>
#include <math.h>

double calculate(double num1, double num2, char operator) {
    switch (operator) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            if (num2 == 0) {
                printf("Error: Division by zero\\n");
                return NAN;
            }
            return num1 / num2;
        default:
            printf("Error: Invalid operator\\n");
            return NAN;
    }
}

int main() {
    // Test case 1: Addition
    printf("Test 1: 10 + 5 = %.2f\\n", calculate(10, 5, '+'));
    
    // Test case 2: Subtraction
    printf("Test 2: 20 - 8 = %.2f\\n", calculate(20, 8, '-'));
    
    // Test case 3: Multiplication
    printf("Test 3: 7 * 6 = %.2f\\n", calculate(7, 6, '*'));
    
    // Test case 4: Division
    printf("Test 4: 50 / 2 = %.2f\\n", calculate(50, 2, '/'));
    
    // Test case 5: Division by zero
    printf("Test 5: 10 / 0 = %.2f\\n", calculate(10, 0, '/'));
    
    return 0;
}
