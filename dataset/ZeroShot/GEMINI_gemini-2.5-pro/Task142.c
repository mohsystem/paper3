#include <stdio.h>
#include <stdlib.h> // For labs()

/**
 * @brief Calculates the greatest common divisor (GCD) of two numbers using the iterative Euclidean algorithm.
 * 
 * This function is secure against stack overflow by being iterative and handles edge cases
 * such as zero and negative inputs by taking their absolute values. It uses long to
 * mitigate integer overflow for a wider range of inputs compared to int.
 * 
 * @param a The first number.
 * @param b The second number.
 * @return The greatest common divisor of a and b.
 */
long gcd(long a, long b) {
    // The GCD is conventionally defined for positive integers.
    // gcd(a, b) is equal to gcd(|a|, |b|).
    a = labs(a);
    b = labs(b);

    // Iterative Euclidean algorithm
    while (b != 0) {
        long temp = b;
        b = a % b;
        a = temp;
    }

    // The algorithm naturally handles cases like gcd(n, 0) = n and gcd(0, 0) = 0.
    return a;
}

int main() {
    printf("Running test cases for C GCD:\n");

    // Test Case 1: Standard case
    long a1 = 48, b1 = 18;
    printf("GCD(%ld, %ld) = %ld\n", a1, b1, gcd(a1, b1));

    // Test Case 2: Coprime numbers (GCD is 1)
    long a2 = 101, b2 = 103;
    printf("GCD(%ld, %ld) = %ld\n", a2, b2, gcd(a2, b2));

    // Test Case 3: One number is zero
    long a3 = 56, b3 = 0;
    printf("GCD(%ld, %ld) = %ld\n", a3, b3, gcd(a3, b3));

    // Test Case 4: Negative inputs
    long a4 = -60, b4 = 48;
    printf("GCD(%ld, %ld) = %ld\n", a4, b4, gcd(a4, b4));

    // Test Case 5: Both numbers are zero
    long a5 = 0, b5 = 0;
    printf("GCD(%ld, %ld) = %ld\n", a5, b5, gcd(a5, b5));

    return 0;
}