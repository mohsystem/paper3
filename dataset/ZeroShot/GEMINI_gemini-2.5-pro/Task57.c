#include <stdio.h>

/**
 * Calculates the factorial of a non-negative integer securely.
 *
 * @param n The non-negative integer.
 * @return The factorial of n as a long long. Returns -1 as an error code 
 *         if n is negative or if the result would overflow (n > 20).
 */
long long calculateFactorial(int n) {
    // 1. Input validation: Factorial is not defined for negative numbers.
    if (n < 0) {
        return -1; // Error code for negative input
    }
    
    // 2. Input validation: Prevent overflow for long long.
    // 20! is the largest factorial that fits in a 64-bit signed long long.
    if (n > 20) {
        return -1; // Error code for overflow
    }

    // 3. Use an iterative approach to avoid stack overflow.
    long long result = 1LL;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    // 5 test cases
    int testCases[] = {0, 1, 5, 20, -5};
    
    printf("C Factorial Test Cases:\n");
    for (int i = 0; i < 5; ++i) {
        int test = testCases[i];
        long long factorial = calculateFactorial(test);
        
        if (factorial == -1) {
            printf("Error for input %d: Invalid input or overflow.\n", test);
        } else {
            // %lld is the format specifier for long long
            printf("Factorial of %d is %lld\n", test, factorial);
        }
    }
    
    return 0;
}