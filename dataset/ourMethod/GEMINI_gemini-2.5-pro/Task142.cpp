#include <iostream>
#include <cmath>   // For std::abs
#include <utility> // For std::swap

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
    a = std::abs(a);
    b = std::abs(b);
    
    while (b) {
        a %= b;
        std::swap(a, b);
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

    for (const auto& test_case : test_cases) {
        long long num1 = test_case[0];
        long long num2 = test_case[1];
        long long result = gcd(num1, num2);
        std::cout << "GCD of " << num1 << " and " << num2 << " is: " << result << std::endl;
    }

    return 0;
}