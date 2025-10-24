#include <stdio.h>

/**
 * @brief Finds the greatest common divisor (GCD) of two numbers using the Euclidean algorithm.
 * 
 * @param a The first number.
 * @param b The second number.
 * @return The greatest common divisor of a and b.
 */
int gcd(int a, int b) {
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

int main() {
    // Test cases
    printf("C Test Cases:\n");
    printf("GCD(48, 18) = %d\n", gcd(48, 18));         // Expected: 6
    printf("GCD(101, 103) = %d\n", gcd(101, 103));   // Expected: 1
    printf("GCD(56, 98) = %d\n", gcd(56, 98));       // Expected: 14
    printf("GCD(270, 192) = %d\n", gcd(270, 192));   // Expected: 6
    printf("GCD(15, 0) = %d\n", gcd(15, 0));         // Expected: 15
    return 0;
}