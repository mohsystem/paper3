#include <iostream>
#include <stdexcept>

/**
 * Performs a basic arithmetic operation on two numbers.
 *
 * @param num1 The first number.
 * @param num2 The second number.
 * @param op The operator character (+, -, *, /).
 * @return The result of the operation.
 * @throws std::invalid_argument if the operator is invalid or if division by zero is attempted.
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
            if (num2 == 0) {
                throw std::invalid_argument("Division by zero is not allowed.");
            }
            return num1 / num2;
        default:
            throw std::invalid_argument("Invalid operator.");
    }
}

int main() {
    // Test Case 1: Addition
    try {
        std::cout << "Test 1: 10.5 + 2.5 = " << calculate(10.5, 2.5, '+') << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test 1 Error: " << e.what() << std::endl;
    }

    // Test Case 2: Subtraction
    try {
        std::cout << "Test 2: 20.0 - 5.5 = " << calculate(20.0, 5.5, '-') << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test 2 Error: " << e.what() << std::endl;
    }

    // Test Case 3: Multiplication
    try {
        std::cout << "Test 3: 5.0 * 4.0 = " << calculate(5.0, 4.0, '*') << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test 3 Error: " << e.what() << std::endl;
    }

    // Test Case 4: Division by Zero (Error)
    try {
        std::cout << "Test 4: 10.0 / 0.0 = " << calculate(10.0, 0.0, '/') << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test 4 Error: " << e.what() << std::endl;
    }

    // Test Case 5: Invalid Operator (Error)
    try {
        std::cout << "Test 5: 10.0 % 5.0 = " << calculate(10.0, 5.0, '%') << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test 5 Error: " << e.what() << std::endl;
    }

    return 0;
}