
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

// Function to find n such that sum of cubes from 1 to n equals m
// Returns n if found, -1 otherwise
// Security: All inputs validated, overflow checks at each step
long long findNb(long long m) {
    // Validate input: m must be positive
    // Reject negative values and zero as invalid volumes
    if (m <= 0) {
        return -1;
    }
    
    // Check for values that would cause overflow during calculation
    // Set safe upper bound to prevent overflow in sum accumulation
    if (m > LLONG_MAX / 2) {
        return -1;
    }
    
    long long n = 0;
    long long sum = 0;
    
    // Iterate to find n where sum of cubes equals m
    // Each iteration checks for overflow before computation
    while (sum < m) {
        n++;
        
        // Prevent overflow when computing n^3
        // Maximum safe n is approximately cbrt(LLONG_MAX) = 2097151
        if (n > 2097151LL) {
            return -1;
        }
        
        // Compute cube with intermediate overflow check
        long long n_squared = n * n;
        long long cube = n_squared * n;
        
        // Validate cube computation didn't overflow\n        // If division doesn't recover original value, overflow occurred
        if (n != 0 && cube / n != n_squared) {
            return -1;
        }
        
        // Check overflow before adding cube to sum
        if (sum > LLONG_MAX - cube) {
            return -1;
        }
        
        sum += cube;
        
        // If sum exceeds m, no exact solution exists
        if (sum > m) {
            return -1;
        }
    }
    
    // Return n only if exact match found
    return (sum == m) ? n : -1;
}

int main(void) {
    // Test case 1: Expected output 45
    printf("Test 1: findNb(1071225) = %lld\\n", findNb(1071225LL));
    
    // Test case 2: Expected output -1 (no exact match)
    printf("Test 2: findNb(91716553919377) = %lld\\n", findNb(91716553919377LL));
    
    // Test case 3: Small valid input (1^3 + 2^3 = 9)
    printf("Test 3: findNb(9) = %lld\\n", findNb(9LL));
    
    // Test case 4: Invalid input (negative value)
    printf("Test 4: findNb(-100) = %lld\\n", findNb(-100LL));
    
    // Test case 5: Edge case (single cube)
    printf("Test 5: findNb(1) = %lld\\n", findNb(1LL));
    
    return 0;
}
