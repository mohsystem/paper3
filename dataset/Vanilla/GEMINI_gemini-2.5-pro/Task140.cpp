#include <iostream>
#include <limits> // Required for std::numeric_limits

/**
 * @brief Performs a basic arithmetic operation.
 *
 * @param num1 The first number.
 * @param num2 The second number.
 * @param op The operator (+, -, *, /).
 * @return The result of the operation, or NaN if the operation is invalid.
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
                std::cerr << "Error: Division by zero is not allowed." << std::endl;
                return std::numeric_limits<double>::quiet_NaN();
            }
        default:
            std::cerr << "Error: Invalid operator '" << op << "'." << std::endl;
            return std::numeric_limits<double>::quiet_NaN();
    }
}

int main() {
    // Test Case 1: Addition
    double num1_1 = 10, num2_1 = 5;
    char op1 = '+';
    std::cout << "Test Case 1: " << num1_1 << " " << op1 << " " << num2_1 << " = " << calculate(num1_1, num2_1, op1) << std::endl;

    // Test Case 2: Subtraction
    double num1_2 = 20.5, num2_2 = 10.5;
    char op2 = '-';
    std::cout << "Test Case 2: " << num1_2 << " " << op2 << " " << num2_2 << " = " << calculate(num1_2, num2_2, op2) << std::endl;

    // Test Case 3: Multiplication
    double num1_3 = 7, num2_3 = 8;
    char op3 = '*';
    std::cout << "Test Case 3: " << num1_3 << " " << op3 << " " << num2_3 << " = " << calculate(num1_3, num2_3, op3) << std::endl;

    // Test Case 4: Division
    double num1_4 = 100, num2_4 = 4;
    char op4 = '/';
    std::cout << "Test Case 4: " << num1_4 << " " << op4 << " " << num2_4 << " = " << calculate(num1_4, num2_4, op4) << std::endl;

    // Test Case 5: Division by zero
    double num1_5 = 15, num2_5 = 0;
    char op5 = '/';
    // The error message is printed from the function, and the result will be 'nan'
    std::cout << "Test Case 5: " << num1_5 << " " << op5 << " " << num2_5 << " = " << calculate(num1_5, num2_5, op5) << std::endl;

    return 0;
}