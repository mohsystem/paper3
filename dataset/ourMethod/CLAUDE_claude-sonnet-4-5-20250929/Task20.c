
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>

// Function to find k such that sum of digits raised to consecutive powers equals k*n
// Returns k if found, -1 otherwise
long long digPow(long long n, int p) {
    // Input validation: n and p must be strictly positive
    if (n <= 0 || p <= 0) {
        return -1;
    }
    
    // Convert n to string to extract digits safely
    // Maximum digits for long long is 20 (including sign), +1 for null terminator
    char numStr[32];
    // Use snprintf for safe string formatting with bounds checking
    int written = snprintf(numStr, sizeof(numStr), "%lld", n);
    
    // Validate snprintf success and buffer not truncated
    if (written < 0 || written >= (int)sizeof(numStr)) {
        return -1;
    }
    
    // Ensure string is null-terminated (snprintf guarantees this)
    // and has at least one character
    size_t len = strlen(numStr);
    if (len == 0 || len > 20) {
        return -1;
    }
    
    long long sum = 0;
    int currentPower = p;
    
    // Calculate sum of each digit raised to consecutive powers
    for (size_t i = 0; i < len; i++) {
        // Bounds check: ensure we're within the string\n        if (i >= sizeof(numStr)) {\n            return -1;\n        }\n        \n        // Extract digit and validate it's in range 0-9
        char c = numStr[i];
        if (c < '0' || c > '9') {
            return -1;
        }
        int digit = c - '0';
        
        // Check for potential overflow before power calculation
        // If power is too large, result will overflow
        if (currentPower > 20) {
            return -1;
        }
        
        // Calculate digit^currentPower safely
        double powerResult = pow((double)digit, (double)currentPower);
        
        // Check if result is within long long range
        if (powerResult > (double)LLONG_MAX || powerResult < 0.0) {
            return -1;
        }
        
        long long powerValue = (long long)powerResult;
        
        // Check for overflow before addition
        if (sum > LLONG_MAX - powerValue) {
            return -1;
        }
        
        sum += powerValue;
        currentPower++;
    }
    
    // Check if sum is divisible by n
    // n > 0 already validated, so no division by zero
    if (sum % n == 0) {
        long long k = sum / n;
        return k;
    }
    
    return -1;
}

int main(void) {
    // Test case 1: 89, p=1 -> expected 1
    printf("Test 1: digPow(89, 1) = %lld (expected 1)\\n", digPow(89, 1));
    
    // Test case 2: 92, p=1 -> expected -1
    printf("Test 2: digPow(92, 1) = %lld (expected -1)\\n", digPow(92, 1));
    
    // Test case 3: 695, p=2 -> expected 2
    printf("Test 3: digPow(695, 2) = %lld (expected 2)\\n", digPow(695, 2));
    
    // Test case 4: 46288, p=3 -> expected 51
    printf("Test 4: digPow(46288, 3) = %lld (expected 51)\\n", digPow(46288, 3));
    
    // Test case 5: edge case with 1
    printf("Test 5: digPow(1, 1) = %lld (expected 1)\\n", digPow(1, 1));
    
    return 0;
}
