
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Function to find GCD using Euclidean algorithm
// Security: Uses iterative approach to avoid stack overflow from recursion
// Validates inputs to prevent undefined behavior with negative numbers
long long gcd(long long a, long long b) {
    // Security: Handle negative numbers by converting to absolute values
    // This prevents undefined behavior in modulo operation
    // Use conditional operator to avoid abs() which may have undefined behavior for LLONG_MIN
    if (a < 0) {
        // Security: Check for LLONG_MIN which cannot be negated safely
        if (a == LLONG_MIN) {
            fprintf(stderr, "Error: Number too large to process\\n");
            return -1;
        }
        a = -a;
    }
    if (b < 0) {
        // Security: Check for LLONG_MIN which cannot be negated safely
        if (b == LLONG_MIN) {
            fprintf(stderr, "Error: Number too large to process\\n");
            return -1;
        }
        b = -b;
    }
    
    // Security: Handle edge case where both numbers are zero
    if (a == 0 && b == 0) {
        fprintf(stderr, "Error: GCD of 0 and 0 is undefined\\n");
        return -1;
    }
    
    // Euclidean algorithm - iterative to prevent stack overflow
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    
    return a;
}

int main(void) {
    long long result;
    
    // Test case 1: Basic positive numbers
    printf("Test case 1: Basic positive numbers\\n");
    result = gcd(48, 18);
    if (result != -1) {
        printf("GCD of 48 and 18 is: %lld\\n\\n", result);
    }
    
    // Test case 2: One number is zero
    printf("Test case 2: One number is zero\\n");
    result = gcd(0, 5);
    if (result != -1) {
        printf("GCD of 0 and 5 is: %lld\\n\\n", result);
    }
    
    // Test case 3: Negative numbers
    printf("Test case 3: Negative numbers\\n");
    result = gcd(-36, 24);
    if (result != -1) {
        printf("GCD of -36 and 24 is: %lld\\n\\n", result);
    }
    
    // Test case 4: Large numbers
    printf("Test case 4: Large numbers\\n");
    result = gcd(1071, 462);
    if (result != -1) {
        printf("GCD of 1071 and 462 is: %lld\\n\\n", result);
    }
    
    // Test case 5: Prime numbers (coprime)
    printf("Test case 5: Prime numbers (coprime)\\n");
    result = gcd(17, 19);
    if (result != -1) {
        printf("GCD of 17 and 19 is: %lld\\n\\n", result);
    }
    
    return 0;
}
