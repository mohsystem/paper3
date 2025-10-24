
#include <iostream>
#include <limits>

class Task140 {
public:
    static bool calculate(double num1, double num2, char op, double& result) {
        switch (op) {
            case '+':
                result = num1 + num2;
                return true;
            case '-':
                result = num1 - num2;
                return true;
            case '*':
                result = num1 * num2;
                return true;
            case '/':
                if (num2 == 0) {
                    std::cout << "Error: Division by zero" << std::endl;
                    return false;
                }
                result = num1 / num2;
                return true;
            default:
                std::cout << "Error: Invalid operator" << std::endl;
                return false;
        }
    }
};

int main() {
    double result;
    
    // Test case 1: Addition
    if (Task140::calculate(10, 5, '+', result)) {
        std::cout << "Test 1 (10 + 5): " << result << std::endl;
    }
    
    // Test case 2: Subtraction
    if (Task140::calculate(10, 5, '-', result)) {
        std::cout << "Test 2 (10 - 5): " << result << std::endl;
    }
    
    // Test case 3: Multiplication
    if (Task140::calculate(10, 5, '*', result)) {
        std::cout << "Test 3 (10 * 5): " << result << std::endl;
    }
    
    // Test case 4: Division
    if (Task140::calculate(10, 5, '/', result)) {
        std::cout << "Test 4 (10 / 5): " << result << std::endl;
    }
    
    // Test case 5: Division by zero
    std::cout << "Test 5 (10 / 0): ";
    Task140::calculate(10, 0, '/', result);
    
    return 0;
}
