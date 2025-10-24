#include <stdio.h>
#include <stdlib.h> // For abs()

/**
 * @brief Given two integers a and b, which can be positive or negative,
 * find the sum of all the integers between and including them and return it.
 * If the two numbers are equal return a or b.
 * 
 * @param a The first integer.
 * @param b The second integer.
 * @return The sum of all integers between and including a and b.
 */
int getSum(int a, int b) {
    // Use the formula for the sum of an arithmetic series:
    // sum = (number of terms / 2) * (first term + last term)
    // number of terms = abs(a - b) + 1
    // first term + last term = a + b
    // Use long long for the intermediate multiplication to prevent integer overflow.
    return (int)(((long long)a + b) * (abs(a - b) + 1) / 2);
}

int main() {
    // Test cases
    printf("%d\n", getSum(1, 0));   // Expected: 1
    printf("%d\n", getSum(1, 2));   // Expected: 3
    printf("%d\n", getSum(1, 1));   // Expected: 1
    printf("%d\n", getSum(-1, 0));  // Expected: -1
    printf("%d\n", getSum(-1, 2));  // Expected: 2
    return 0;
}