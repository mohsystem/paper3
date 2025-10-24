
#include <iostream>
#include <string>
#include <limits>
#include <cerrno>
#include <cstring>

// Function to calculate factorial with overflow protection
// Returns true on success, false on overflow or invalid input
bool calculateFactorial(int n, unsigned long long& result) {
    // Validate input: factorial is only defined for non-negative integers
    if (n < 0) {
        return false;
    }
    
    // Handle base cases
    if (n == 0 || n == 1) {
        result = 1;
        return true;
    }
    
    // Limit input to prevent overflow (20! is max for unsigned long long)
    if (n > 20) {
        return false;
    }
    
    result = 1;
    
    // Calculate factorial with overflow checking at each step
    for (int i = 2; i <= n; i++) {
        // Check for overflow before multiplication
        if (result > ULLONG_MAX / i) {
            return false;
        }
        result *= i;
    }
    
    return true;
}

int main() {
    // Test cases
    const int testCases[5] = {5, 0, 10, 15, 20};
    
    for (int i = 0; i < 5; i++) {
        int input = testCases[i];
        unsigned long long factorial = 0;
        
        if (calculateFactorial(input, factorial)) {
            std::cout << "Factorial of " << input << " = " << factorial << std::endl;
        } else {
            std::cout << "Error: Cannot calculate factorial of " << input 
                      << " (invalid input or overflow)" << std::endl;
        }
    }
    
    return 0;
}
