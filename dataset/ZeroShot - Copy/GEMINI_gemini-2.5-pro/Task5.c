#include <stdio.h>

/**
 * Calculates the multiplicative persistence of a positive number.
 *
 * @param num A non-negative long long integer.
 * @return The multiplicative persistence of num, or -1 if the input is negative.
 */
int persistence(long long num) {
    // Secure coding: Validate input. In C, returning an error code is a common pattern.
    if (num < 0) {
        return -1; // Error code for negative input
    }

    int count = 0;
    while (num >= 10) {
        count++;
        long long product = 1;
        long long temp = num;
        while (temp > 0) {
            product *= temp % 10;
            temp /= 10;
        }
        num = product;
    }
    return count;
}

int main() {
    // 5 Test Cases
    printf("persistence(39) -> %d\n", persistence(39));      // Expected: 3
    printf("persistence(999) -> %d\n", persistence(999));    // Expected: 4
    printf("persistence(4) -> %d\n", persistence(4));        // Expected: 0
    printf("persistence(25) -> %d\n", persistence(25));      // Expected: 2
    printf("persistence(679) -> %d\n", persistence(679));    // Expected: 5

    // Example of checking the error code for a bad input
    long long bad_input = -10;
    int result = persistence(bad_input);
    if (result == -1) {
        printf("persistence(%lld) -> Error: Input cannot be negative.\n", bad_input);
    }
    
    return 0;
}