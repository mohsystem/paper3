#include <stdio.h>

/**
 * @brief Given two integers a and b, which can be positive or negative,
 * find the sum of all the integers between and including them and return it.
 * If the two numbers are equal return a or b.
 * 
 * @param a An integer.
 * @param b An integer.
 * @return The sum of all integers between a and b (inclusive).
 */
long long getSum(int a, int b) {
    // If the numbers are the same, return one of them.
    if (a == b) {
        return a;
    }

    // Use long long to prevent potential integer overflow during calculation.
    long long min_val = (a < b) ? a : b;
    long long max_val = (a > b) ? a : b;
    
    // Apply the formula for the sum of an arithmetic series:
    // Sum = n/2 * (first + last)
    // where n is the number of terms (max - min + 1)
    return (max_val - min_val + 1) * (min_val + max_val) / 2;
}

int main() {
    // Test cases
    printf("%lld\n", getSum(1, 0));    // Expected: 1
    printf("%lld\n", getSum(1, 2));    // Expected: 3
    printf("%lld\n", getSum(1, 1));    // Expected: 1
    printf("%lld\n", getSum(-1, 0));   // Expected: -1
    printf("%lld\n", getSum(-1, 2));   // Expected: 2
    return 0;
}