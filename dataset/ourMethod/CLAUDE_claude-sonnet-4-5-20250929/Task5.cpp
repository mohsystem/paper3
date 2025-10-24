
#include <iostream>
#include <stdexcept>
#include <limits>

// Function to calculate multiplicative persistence
// Returns the number of times digits must be multiplied until a single digit remains
// Input validation: num must be non-negative to prevent undefined behavior
int persistence(long long num) {
    // Validate input: must be non-negative as per problem statement (positive parameter)
    if (num < 0) {
        throw std::invalid_argument("Input must be non-negative");
    }
    
    // Prevent overflow: limit input to reasonable range
    // Maximum safe value that won't cause overflow during digit multiplication
    if (num > 999999999999999LL) {
        throw std::invalid_argument("Input exceeds maximum safe value");
    }
    
    int count = 0;
    
    // Continue until we reach a single digit (0-9)
    while (num >= 10) {
        long long product = 1;
        
        // Multiply all digits together
        while (num > 0) {
            long long digit = num % 10;
            
            // Check for potential overflow before multiplication
            // This prevents integer overflow vulnerability
            if (product > LLONG_MAX / digit) {
                throw std::overflow_error("Multiplication would cause overflow");
            }
            
            product *= digit;
            num /= 10;
        }
        
        num = product;
        count++;
        
        // Sanity check: prevent infinite loop (though mathematically impossible)
        // This adds defense-in-depth against unexpected behavior
        if (count > 1000) {
            throw std::runtime_error("Exceeded maximum iteration count");
        }
    }
    
    return count;
}

int main() {
    // Test cases with error handling
    try {
        std::cout << "Test 1: persistence(39) = " << persistence(39) << " (expected: 3)" << std::endl;
        std::cout << "Test 2: persistence(999) = " << persistence(999) << " (expected: 4)" << std::endl;
        std::cout << "Test 3: persistence(4) = " << persistence(4) << " (expected: 0)" << std::endl;
        std::cout << "Test 4: persistence(25) = " << persistence(25) << " (expected: 2)" << std::endl;
        std::cout << "Test 5: persistence(77) = " << persistence(77) << " (expected: 4)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
