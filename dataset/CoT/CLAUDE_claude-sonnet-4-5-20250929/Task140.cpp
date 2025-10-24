
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>

class Task140 {
public:
    static double calculate(double num1, double num2, const std::string& op) {
        // Validate operator input (whitelist approach)
        if (op.length() != 1) {
            throw std::invalid_argument("Invalid operator");
        }
        
        char operation = op[0];
        
        switch (operation) {
            case '+':
                return num1 + num2;
            case '-':
                return num1 - num2;
            case '*':
                return num1 * num2;
            case '/':
                // Secure division with zero-check
                if (num2 == 0 || std::abs(num2) < 1e-10) {
                    throw std::runtime_error("Division by zero is not allowed");
                }
                return num1 / num2;
            default:
                throw std::invalid_argument("Invalid operator. Use +, -, *, or /");
        }
    }
};

int main() {
    // Test case 1: Addition
    try {
        std::cout << "Test 1: 10 + 5 = " << Task140::calculate(10, 5, "+") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test case 2: Subtraction
    try {
        std::cout << "Test 2: 20 - 8 = " << Task140::calculate(20, 8, "-") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test case 3: Multiplication
    try {
        std::cout << "Test 3: 7 * 6 = " << Task140::calculate(7, 6, "*") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test case 4: Division
    try {
        std::cout << "Test 4: 15 / 3 = " << Task140::calculate(15, 3, "/") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Division by zero (error handling)
    try {
        std::cout << "Test 5: 10 / 0 = " << Task140::calculate(10, 0, "/") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 Error: " << e.what() << std::endl;
    }
    
    return 0;
}
