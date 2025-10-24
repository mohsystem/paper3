#include <iostream>
#include <algorithm> // For std::min and std::max

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
    long long min_val = std::min(a, b);
    long long max_val = std::max(a, b);
    
    // Formula for the sum of an arithmetic series: n * (first + last) / 2
    // where n is the number of terms (max - min + 1).
    return (max_val - min_val + 1) * (min_val + max_val) / 2;
}

int main() {
    // Test Cases
    std::cout << getSum(1, 0) << std::endl;    // Expected output: 1
    std::cout << getSum(1, 2) << std::endl;    // Expected output: 3
    std::cout << getSum(0, 1) << std::endl;    // Expected output: 1
    std::cout << getSum(1, 1) << std::endl;    // Expected output: 1
    std::cout << getSum(-1, 2) << std::endl;   // Expected output: 2
    
    return 0;
}