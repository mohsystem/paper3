#include <iostream>

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
    std::cout << "C++ Test Cases:" << std::endl;
    std::cout << "GCD(48, 18) = " << gcd(48, 18) << std::endl;         // Expected: 6
    std::cout << "GCD(101, 103) = " << gcd(101, 103) << std::endl;   // Expected: 1
    std::cout << "GCD(56, 98) = " << gcd(56, 98) << std::endl;       // Expected: 14
    std::cout << "GCD(270, 192) = " << gcd(270, 192) << std::endl;   // Expected: 6
    std::cout << "GCD(15, 0) = " << gcd(15, 0) << std::endl;         // Expected: 15
    return 0;
}