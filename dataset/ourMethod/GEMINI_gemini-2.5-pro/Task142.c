#include <stdio.h>
#include <stdlib.h> // For llabs

/**
 * @brief Calculates the greatest common divisor (GCD) of two numbers using the iterative Euclidean algorithm.
 * 
 * This function handles positive, negative, and zero inputs. The result is always non-negative.
 * 
 * @param a The first number.
 * @param b The second number.
 * @return The greatest common divisor of a and b.
 */
long long gcd(long long a, long long b) {
    // The GCD is conventionally a non-negative number.
    // gcd(a, b) = gcd(|a|, |b|).
    a = llabs(a);
    b = llabs(b);
    
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int main() {
    // 5 Test Cases
    long long test_cases[][2] = {
        {54, 24},      // Standard case
        {101, 103},    // Coprime numbers
        {0, 5},        // One number is zero
        {12, 0},       // The other number is zero
        {-48, -18}     // Negative numbers
    };

    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        long long num1 = test_cases[i][0];
        long long num2 = test_cases[i][1];
        long long result = gcd(num1, num2);
        printf("GCD of %lld and %lld is: %lld\n", num1, num2, result);
    }

    return 0;
}