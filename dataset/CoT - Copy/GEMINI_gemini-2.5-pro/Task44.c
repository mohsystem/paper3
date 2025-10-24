#include <stdio.h>
#include <stdlib.h> // For exit()

/**
 * @brief Clears the standard input buffer.
 * Useful after a failed scanf to prevent infinite loops.
 */
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * @brief Performs a basic arithmetic operation after validating inputs.
 * 
 * @param num1 The first integer.
 * @param num2 The second integer.
 * @param op The character representing the operation (+, -, *, /).
 */
void performOperation(int num1, int num2, char op) {
    double result = 0.0;
    switch (op) {
        case '+':
            result = (double)num1 + num2;
            break;
        case '-':
            result = (double)num1 - num2;
            break;
        case '*':
            result = (double)num1 * num2;
            break;
        case '/':
            // Security: Validate against division by zero.
            if (num2 == 0) {
                printf("Error: Division by zero is not allowed.\n");
                return;
            }
            result = (double)num1 / num2;
            break;
        default:
            // Security: Validate the operator is one of the allowed characters.
            printf("Error: Invalid operator '%c'.\n", op);
            return;
    }
    printf("%d %c %d = %.2f\n", num1, op, num2, result);
}

// Dummy struct to satisfy potential class naming requirements, not idiomatic for C
typedef struct {
    // No members needed, just a namespace substitute
} Task44;

void run_tests() {
    // --- Test Cases ---
    printf("--- Running Automated Test Cases ---\n");

    // Test Case 1: Addition
    printf("Test Case 1 (Addition):\n");
    printf("Input: 10, 5, '+'\n");
    printf("Output: ");
    performOperation(10, 5, '+');
    printf("\n");

    // Test Case 2: Subtraction
    printf("Test Case 2 (Subtraction):\n");
    printf("Input: 10, 15, '-'\n");
    printf("Output: ");
    performOperation(10, 15, '-');
    printf("\n");

    // Test Case 3: Valid Division
    printf("Test Case 3 (Valid Division):\n");
    printf("Input: 20, 4, '/'\n");
    printf("Output: ");
    performOperation(20, 4, '/');
    printf("\n");

    // Test Case 4: Invalid Division (by zero)
    printf("Test Case 4 (Division by Zero):\n");
    printf("Input: 10, 0, '/'\n");
    printf("Output: ");
    performOperation(10, 0, '/');
    printf("\n");

    // Test Case 5: Invalid Operator
    printf("Test Case 5 (Invalid Operator):\n");
    printf("Input: 7, 8, '%%'\n");
    printf("Output: ");
    performOperation(7, 8, '%');
    printf("\n");
}

void run_interactive() {
    printf("\n--- Starting Interactive Session ---\n");
    int num1, num2;
    char op;
    int scan_result;

    // Securely read first integer
    printf("Enter first integer: ");
    // Security: Check scanf return value to ensure an integer was read.
    while ((scan_result = scanf("%d", &num1)) != 1) {
        printf("Error: Invalid input. Please enter an integer: ");
        // Security: Clear the buffer to prevent an infinite loop on bad input.
        clear_input_buffer();
    }
    // After a successful numeric read, clear the rest of the line (e.g., the newline)
    clear_input_buffer();

    // Securely read second integer
    printf("Enter second integer: ");
    while ((scan_result = scanf("%d", &num2)) != 1) {
        printf("Error: Invalid input. Please enter an integer: ");
        clear_input_buffer();
    }
    clear_input_buffer();
    
    // Securely read operator
    printf("Enter an operator (+, -, *, /): ");
    // Security: The leading space in " %c" tells scanf to skip any whitespace,
    // including the newline left by the previous scanf.
    while (scanf(" %c", &op) != 1 || (op != '+' && op != '-' && op != '*' && op != '/')) {
        printf("Error: Invalid operator. Please use +, -, *, or /: ");
        clear_input_buffer();
    }

    printf("Result: ");
    performOperation(num1, num2, op);
}


int main() {
    run_tests();
    // To run interactively, uncomment the following line:
    // run_interactive();
    return 0;
}