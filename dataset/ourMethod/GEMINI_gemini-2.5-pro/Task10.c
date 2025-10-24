#include <stdio.h>

/**
 * @brief Given two integers a and b, which can be positive or negative, find the sum of all the integers between and including them and return it.
 * If the two numbers are equal return a or b.
 *
 * Note: a and b are not ordered!
 *
 * @param a The first integer.
 * @param b The second integer.
 * @return The sum of all integers between a and b, inclusive. Returns a 64-bit integer to prevent overflow.
 */
long long getSum(int a, int b) {
    // Cast to long long to avoid overflow during intermediate calculations.
    long long long_a = a;
    long long long_b = b;

    if (long_a == long_b) {
        return long_a;
    }

    long long min_val;
    long long max_val;

    if (long_a < long_b) {
        min_val = long_a;
        max_val = long_b;
    } else {
        min_val = long_b;
        max_val = long_a;
    }

    // Using the formula for the sum of an arithmetic series:
    // sum = (number of terms) * (first + last) / 2
    // number of terms = max - min + 1
    long long num_terms = max_val - min_val + 1;
    long long sum = (min_val + max_val) * num_terms / 2;

    return sum;
}

int main() {
    // 5 Test cases
    printf("C Test Cases:\n");

    // Test case 1: (1, 0) -> 1
    int a1 = 1, b1 = 0;
    printf("getSum(%d, %d) = %lld\n", a1, b1, getSum(a1, b1));

    // Test case 2: (1, 2) -> 3
    int a2 = 1, b2 = 2;
    printf("getSum(%d, %d) = %lld\n", a2, b2, getSum(a2, b2));

    // Test case 3: (1, 1) -> 1
    int a3 = 1, b3 = 1;
    printf("getSum(%d, %d) = %lld\n", a3, b3, getSum(a3, b3));

    // Test case 4: (-1, 0) -> -1
    int a4 = -1, b4 = 0;
    printf("getSum(%d, %d) = %lld\n", a4, b4, getSum(a4, b4));

    // Test case 5: (-1, 2) -> 2
    int a5 = -1, b5 = 2;
    printf("getSum(%d, %d) = %lld\n", a5, b5, getSum(a5, b5));

    return 0;
}