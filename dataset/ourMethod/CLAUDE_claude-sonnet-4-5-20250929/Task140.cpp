
#include <iostream>
#include <string>
#include <limits>
#include <cmath>
#include <stdexcept>

// Function to perform arithmetic operation with input validation
// Returns the result of the operation
double calculate(double num1, double num2, char op) {
    // Validate operator input - only allow specific operators
    if (op != '+' && op != '-' && op != '*' && op != '/') {
        throw std::invalid_argument("Invalid operator. Only +, -, *, / are allowed.");
    }
    
    // Check for potential overflow before operations
    // Prevent undefined behavior with extreme values
    if (std::isnan(num1) || std::isnan(num2) || 
        std::isinf(num1) || std::isinf(num2)) {
        throw std::invalid_argument("Invalid number: NaN or Infinity not allowed.");
    }
    
    double result = 0.0;
    
    switch(op) {
        case '+':
            // Check for overflow in addition
            if ((num2 > 0 && num1 > std::numeric_limits<double>::max() - num2) ||
                (num2 < 0 && num1 < std::numeric_limits<double>::lowest() - num2)) {
                throw std::overflow_error("Addition overflow detected.");
            }
            result = num1 + num2;
            break;
            
        case '-':
            // Check for overflow in subtraction
            if ((num2 < 0 && num1 > std::numeric_limits<double>::max() + num2) ||
                (num2 > 0 && num1 < std::numeric_limits<double>::lowest() + num2)) {
                throw std::overflow_error("Subtraction overflow detected.");
            }
            result = num1 - num2;
            break;
            
        case '*':
            // Check for overflow in multiplication
            if (num1 != 0.0 && num2 != 0.0) {
                if (std::abs(num1) > std::numeric_limits<double>::max() / std::abs(num2)) {
                    throw std::overflow_error("Multiplication overflow detected.");
                }
            }
            result = num1 * num2;
            break;
            
        case '/':
            // Critical: Prevent division by zero
            if (num2 == 0.0) {
                throw std::invalid_argument("Division by zero is not allowed.");
            }
            result = num1 / num2;
            break;
            
        default:
            // Should never reach here due to validation above
            throw std::invalid_argument("Unknown operator.");
    }
    
    // Validate result is not NaN or Infinity after operation
    if (std::isnan(result) || std::isinf(result)) {
        throw std::overflow_error("Operation resulted in invalid value.");
    }
    
    return result;
}

int main() {
    // Test cases with comprehensive validation
    std::cout << "Test Case 1: 10 + 5" << std::endl;
    try {
        double result = calculate(10.0, 5.0, '+');
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 2: 20 - 8" << std::endl;
    try {
        double result = calculate(20.0, 8.0, '-');
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 3: 7 * 6" << std::endl;
    try {
        double result = calculate(7.0, 6.0, '*');
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 4: 15 / 3" << std::endl;
    try {
        double result = calculate(15.0, 3.0, '/');
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 5: 10 / 0 (Division by zero)" << std::endl;
    try {
        double result = calculate(10.0, 0.0, '/');
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        // Expected error - handle gracefully
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
