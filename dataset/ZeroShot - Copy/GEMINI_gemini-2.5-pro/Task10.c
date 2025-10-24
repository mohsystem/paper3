#include <stdio.h>

/**
 * Calculates the sum of all integers between and including two given integers.
 * Uses a mathematical formula for an arithmetic series to efficiently compute the sum
 * and uses 'long long' to prevent potential integer overflow for large ranges.
 *
 * @param a The first integer.
 * @param b The second integer.
 * @return The sum of all integers between a and b, inclusive.
 */
long long getSum(int a, int b) {
    if (a == b) {
        return a;
    }

    // Cast to long long for calculations to prevent overflow.
    long long min_val = a < b ? a : b;
    long long max_val = a > b ? a : b;
    
    // Formula for the sum of an arithmetic series: n * (first + last) / 2
    // where n is the number of terms (max - min + 1).
    return (max_val - min_val + 1) * (min_val + max_val) / 2;
}

int main() {
    // Test Cases
    printf("%lld\n", getSum(1, 0));    // Expected output: 1
    printf("%lld\n", getSum(1, 2));    // Expected output: 3
    printf("%lld\n", getSum(0, 1));    // Expected output: 1
    printf("%lld\n", getSum(1, 1));    // Expected output: 1
    printf("%lld\n", getSum(-1, 2));   // Expected output: 2

    return 0;
}