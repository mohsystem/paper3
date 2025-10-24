#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>

int digPow(int n, int p) {
    if (n <= 0 || p <= 0) {
        return -1; // As per problem statement, n and p are strictly positive
    }
    
    char snum[12]; // Buffer for string representation of n. 12 is safe for a 32-bit int.
    
    int len = snprintf(snum, sizeof(snum), "%d", n);

    if (len < 0 || (size_t)len >= sizeof(snum)) {
        return -1; // Error in conversion
    }

    long long sum = 0;
    for (int i = 0; i < len; i++) {
        int digit = snum[i] - '0';
        long long term = (long long)round(pow(digit, p));
        
        // A simple overflow check
        if (sum > LLONG_MAX - term) {
            // Handle potential overflow
            return -1;
        }
        
        sum += term;
        p++;
    }
    
    if (sum % n == 0) {
        return (int)(sum / n);
    } else {
        return -1;
    }
}

int main() {
    // Test case 1
    printf("Test Case 1: n=89, p=1 -> Result: %d (Expected: 1)\n", digPow(89, 1));
    // Test case 2
    printf("Test Case 2: n=92, p=1 -> Result: %d (Expected: -1)\n", digPow(92, 1));
    // Test case 3
    printf("Test Case 3: n=695, p=2 -> Result: %d (Expected: 2)\n", digPow(695, 2));
    // Test case 4
    printf("Test Case 4: n=46288, p=3 -> Result: %d (Expected: 51)\n", digPow(46288, 3));
    // Test case 5
    printf("Test Case 5: n=135, p=1 -> Result: %d (Expected: 1)\n", digPow(135, 1));
    
    return 0;
}