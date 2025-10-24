#include <stdio.h>

/**
 * Calculates the factorial of a non-negative integer.
 *
 * @param n The number to calculate the factorial of.
 * @return The factorial of n, or -1 if n is negative.
 */
long long factorial(int n) {
    if (n < 0) {
        // Factorial is not defined for negative numbers
        return -1;
    }
    if (n == 0) {
        return 1;
    }
    long long result = 1;
    for (int i = 1; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    printf("C Factorial Test Cases:\n");
    int testCases[] = {0, 1, 5, 10, 15};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < numTestCases; ++i) {
        int test = testCases[i];
        printf("Factorial of %d is %lld\n", test, factorial(test));
    }
    return 0;
}