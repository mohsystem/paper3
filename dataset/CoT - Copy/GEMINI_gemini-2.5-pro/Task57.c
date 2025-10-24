#include <stdio.h>
#include <errno.h> // For errno and error constants

/**
 * Calculates the factorial of a non-negative integer.
 *
 * @param n The non-negative integer. Input is limited to 20
 *          to prevent long long overflow.
 * @return The factorial of n. On error, returns -1 and sets errno.
 *         - errno = EDOM if n is negative (Domain Error).
 *         - errno = ERANGE if n > 20 (Range Error due to overflow).
 */
long long factorial(int n) {
    if (n < 0) {
        errno = EDOM; // Domain error
        return -1;
    }
    if (n > 20) {
        // 21! overflows a 64-bit long long
        errno = ERANGE; // Range error
        return -1;
    }
    if (n == 0) {
        return 1LL;
    }

    long long result = 1LL;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    int testCases[] = {0, 1, 5, 12, 20, -1, 21};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int testCase = testCases[i];
        errno = 0; // Reset errno before the call
        long long result = factorial(testCase);
        
        if (result == -1 && errno != 0) {
            fprintf(stderr, "Error calculating factorial for %d: ", testCase);
            // perror prints the string you give it, followed by an error message based on errno
            perror(""); 
        } else {
            // In C, %lld is the format specifier for long long
            printf("Factorial of %d is: %lld\n", testCase, result);
        }
    }
    
    return 0;
}