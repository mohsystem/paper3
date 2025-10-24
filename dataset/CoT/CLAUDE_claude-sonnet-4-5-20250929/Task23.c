
#include <stdio.h>
#include <math.h>
#include <limits.h>

long long findNb(long long m) {
    // Input validation
    if (m <= 0) {
        return -1;
    }
    
    long long n = 0;
    long long sum = 0;
    
    // Iterate until sum equals or exceeds m
    while (sum < m) {
        n++;
        // Check for potential overflow
        if (n > cbrt(LLONG_MAX)) {
            return -1;
        }
        long long cube = n * n * n;
        // Check if adding cube would overflow
        if (sum > LLONG_MAX - cube) {
            return -1;
        }
        sum += cube;
    }
    
    // Return n if exact match, otherwise -1
    return (sum == m) ? n : -1;
}

int main() {
    // Test cases
    printf("Test 1: %lld (Expected: 45)\\n", findNb(1071225));
    printf("Test 2: %lld (Expected: -1)\\n", findNb(91716553919377LL));
    printf("Test 3: %lld (Expected: 2022)\\n", findNb(4183059834009LL));
    printf("Test 4: %lld (Expected: -1)\\n", findNb(24723578342962LL));
    printf("Test 5: %lld (Expected: 5)\\n", findNb(135));
    
    return 0;
}
