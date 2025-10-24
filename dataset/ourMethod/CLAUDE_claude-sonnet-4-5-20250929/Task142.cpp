
#include <iostream>
#include <limits>
#include <stdexcept>

// Function to find GCD using Euclidean algorithm
// Security: Uses iterative approach to avoid stack overflow from recursion
// Validates inputs to prevent undefined behavior with negative numbers
long long gcd(long long a, long long b) {
    // Security: Handle negative numbers by converting to absolute values
    // This prevents undefined behavior in modulo operation
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    
    // Security: Handle edge case where both numbers are zero
    if (a == 0 && b == 0) {
        throw std::invalid_argument("GCD of 0 and 0 is undefined");
    }
    
    // Euclidean algorithm - iterative to prevent stack overflow
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    
    return a;
}

int main() {
    // Test case 1: Basic positive numbers
    try {
        long long num1 = 48, num2 = 18;
        std::cout << "GCD of " << num1 << " and " << num2 << " is: " << gcd(num1, num2) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 2: One number is zero
    try {
        long long num1 = 0, num2 = 5;
        std::cout << "GCD of " << num1 << " and " << num2 << " is: " << gcd(num1, num2) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 3: Negative numbers
    try {
        long long num1 = -36, num2 = 24;
        std::cout << "GCD of " << num1 << " and " << num2 << " is: " << gcd(num1, num2) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 4: Large numbers
    try {
        long long num1 = 1071, num2 = 462;
        std::cout << "GCD of " << num1 << " and " << num2 << " is: " << gcd(num1, num2) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Prime numbers (coprime)
    try {
        long long num1 = 17, num2 = 19;
        std::cout << "GCD of " << num1 << " and " << num2 << " is: " << gcd(num1, num2) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
