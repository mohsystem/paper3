
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

long long digPow(long long n, int p) {
    // Input validation to prevent security issues
    if (n <= 0 || p <= 0) {
        return -1;
    }
    
    // Convert n to string to extract digits safely
    char nStr[32];
    snprintf(nStr, sizeof(nStr), "%lld", n);
    long long sum = 0;
    
    // Calculate sum of digits raised to consecutive powers
    size_t len = strlen(nStr);
    for (size_t i = 0; i < len; i++) {
        // Validate character is a digit
        if (!isdigit(nStr[i])) {
            return -1;
        }
        
        int digit = nStr[i] - '0';
        int power = p + i;
        
        // Calculate power safely
        long long powResult = (long long)pow(digit, power);
        
        // Check for overflow before addition
        if (sum > LLONG_MAX - powResult) {
            return -1;
        }
        sum += powResult;
    }
    
    // Check if sum is divisible by n
    if (sum % n == 0) {
        return sum / n;
    }
    
    return -1;
}

int main() {
    // Test case 1
    printf("Test 1: n=89, p=1 -> %lld\\n", digPow(89, 1)); // Expected: 1
    
    // Test case 2
    printf("Test 2: n=92, p=1 -> %lld\\n", digPow(92, 1)); // Expected: -1
    
    // Test case 3
    printf("Test 3: n=695, p=2 -> %lld\\n", digPow(695, 2)); // Expected: 2
    
    // Test case 4
    printf("Test 4: n=46288, p=3 -> %lld\\n", digPow(46288, 3)); // Expected: 51
    
    // Test case 5
    printf("Test 5: n=1, p=1 -> %lld\\n", digPow(1, 1)); // Expected: 1
    
    return 0;
}
