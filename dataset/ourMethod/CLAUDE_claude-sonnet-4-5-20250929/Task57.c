
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

// Function to calculate factorial with overflow protection
// Returns true on success, false on overflow or invalid input
bool calculateFactorial(int n, unsigned long long* result) {
    // Validate output pointer to prevent null pointer dereference
    if (result == NULL) {
        return false;
    }
    
    // Initialize output to safe default
    *result = 0;
    
    // Validate input: factorial is only defined for non-negative integers
    if (n < 0) {
        return false;
    }
    
    // Handle base cases
    if (n == 0 || n == 1) {
        *result = 1;
        return true;
    }
    
    // Limit input to prevent overflow (20! is max for unsigned long long)
    if (n > 20) {
        return false;
    }
    
    *result = 1;
    
    // Calculate factorial with overflow checking at each step
    for (int i = 2; i <= n; i++) {
        // Check for overflow before multiplication
        if (*result > ULLONG_MAX / i) {
            *result = 0;
            return false;
        }
        *result *= i;
    }
    
    return true;
}

int main(void) {
    // Test cases array - bounds are known and safe
    const int testCases[5] = {5, 0, 10, 15, 20};
    
    // Process each test case with bounds checking
    for (int i = 0; i < 5; i++) {
        int input = testCases[i];
        unsigned long long factorial = 0;
        
        // Call factorial function with output pointer
        if (calculateFactorial(input, &factorial)) {
            // Safe formatted output with validation
            printf("Factorial of %d = %llu\\n", input, factorial);
        } else {
            // Error case - clear error message
            printf("Error: Cannot calculate factorial of %d (invalid input or overflow)\\n", input);
        }
    }
    
    return 0;
}
