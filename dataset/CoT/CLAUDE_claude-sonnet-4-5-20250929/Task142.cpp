
#include <iostream>
#include <cstdlib>
#include <stdexcept>

// Function to find GCD using Euclidean algorithm
long long findGCD(long long a, long long b) {
    // Input validation - convert negative numbers to positive
    a = std::abs(a);
    b = std::abs(b);
    
    // Handle edge case where both numbers are 0
    if (a == 0 && b == 0) {
        throw std::invalid_argument("GCD is undefined for both numbers being zero");
    }
    
    // Euclidean algorithm
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    
    return a;
}

int main() {
    try {
        // Test case 1: Normal positive numbers
        std::cout << "Test 1 - GCD(48, 18): " << findGCD(48, 18) << std::endl;
        
        // Test case 2: One number is 0
        std::cout << "Test 2 - GCD(0, 5): " << findGCD(0, 5) << std::endl;
        
        // Test case 3: Same numbers
        std::cout << "Test 3 - GCD(100, 100): " << findGCD(100, 100) << std::endl;
        
        // Test case 4: Prime numbers (coprime)
        std::cout << "Test 4 - GCD(17, 19): " << findGCD(17, 19) << std::endl;
        
        // Test case 5: Large numbers with negative input
        std::cout << "Test 5 - GCD(-270, 192): " << findGCD(-270, 192) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
