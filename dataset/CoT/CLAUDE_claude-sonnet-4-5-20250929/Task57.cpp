
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

class Task57 {
public:
    // Using string to handle very large factorials
    static std::string calculateFactorial(int n) {
        // Input validation to prevent negative numbers
        if (n < 0) {
            throw std::invalid_argument("Factorial is not defined for negative numbers");
        }
        
        // Check for reasonable upper limit to prevent resource exhaustion
        if (n > 10000) {
            throw std::invalid_argument("Input too large - maximum value is 10000");
        }
        
        // Base cases
        if (n == 0 || n == 1) {
            return "1";
        }
        
        // Use vector to store large number digits
        std::vector<int> result;
        result.push_back(1);
        
        // Calculate factorial by multiplying iteratively
        for (int x = 2; x <= n; x++) {
            int carry = 0;
            for (size_t i = 0; i < result.size(); i++) {
                int prod = result[i] * x + carry;
                result[i] = prod % 10;
                carry = prod / 10;
            }
            
            while (carry > 0) {
                result.push_back(carry % 10);
                carry = carry / 10;
            }
        }
        
        // Convert vector to string (reverse order)
        std::string factorialStr = "";
        for (int i = result.size() - 1; i >= 0; i--) {
            factorialStr += std::to_string(result[i]);
        }
        
        return factorialStr;
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1: Factorial of 0 = " << Task57::calculateFactorial(0) << std::endl;
    std::cout << "Test Case 2: Factorial of 1 = " << Task57::calculateFactorial(1) << std::endl;
    std::cout << "Test Case 3: Factorial of 5 = " << Task57::calculateFactorial(5) << std::endl;
    std::cout << "Test Case 4: Factorial of 10 = " << Task57::calculateFactorial(10) << std::endl;
    std::cout << "Test Case 5: Factorial of 20 = " << Task57::calculateFactorial(20) << std::endl;
    
    // Test error handling
    try {
        std::cout << "Test Case 6 (Error): Factorial of -5" << std::endl;
        Task57::calculateFactorial(-5);
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
