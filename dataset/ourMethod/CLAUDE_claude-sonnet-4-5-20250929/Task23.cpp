
#include <iostream>
#include <cstdint>
#include <limits>

// Function to find n such that sum of cubes from 1 to n equals m
// Returns n if found, -1 otherwise
// Security: Input validation prevents integer overflow and negative values
long long findNb(long long m) {
    // Validate input: m must be positive
    // Prevent negative values and zero which are invalid volumes
    if (m <= 0) {
        return -1;
    }
    
    // Check for potential overflow in sum calculation
    // Maximum safe value to prevent overflow during accumulation
    if (m > LLONG_MAX / 2) {
        return -1;
    }
    
    long long n = 0;
    long long sum = 0;
    
    // Iterate to find n where sum of cubes equals m
    // We check each step to prevent overflow
    while (sum < m) {
        n++;
        
        // Check for potential overflow before computing n^3
        // If n is too large, n^3 will overflow
        if (n > 2097151) { // cbrt(LLONG_MAX) is approximately 2097151
            return -1;
        }
        
        long long cube = n * n * n;
        
        // Check for overflow before adding to sum
        if (sum > LLONG_MAX - cube) {
            return -1;
        }
        
        sum += cube;
        
        // If sum exceeds m, no exact match exists
        if (sum > m) {
            return -1;
        }
    }
    
    // Return n if exact match found
    return (sum == m) ? n : -1;
}

int main() {
    // Test case 1: Expected output 45
    std::cout << "Test 1: findNb(1071225) = " << findNb(1071225) << std::endl;
    
    // Test case 2: Expected output -1 (no exact match)
    std::cout << "Test 2: findNb(91716553919377) = " << findNb(91716553919377LL) << std::endl;
    
    // Test case 3: Small valid input
    std::cout << "Test 3: findNb(9) = " << findNb(9) << std::endl;
    
    // Test case 4: Invalid input (negative)
    std::cout << "Test 4: findNb(-100) = " << findNb(-100) << std::endl;
    
    // Test case 5: Edge case (1)
    std::cout << "Test 5: findNb(1) = " << findNb(1) << std::endl;
    
    return 0;
}
