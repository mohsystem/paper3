#include <iostream>
#include <cmath> // For std::abs
#include <vector>
#include <utility>

/**
 * @brief Finds the greatest common divisor (GCD) of two integers using the Euclidean algorithm.
 * 
 * @param a The first integer.
 * @param b The second integer.
 * @return The greatest common divisor of a and b.
 */
int findGCD(int a, int b) {
    // The Euclidean algorithm
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    // The GCD is conventionally a non-negative number.
    return std::abs(a);
}

int main() {
    // 5 Test Cases
    std::vector<std::pair<int, int>> testCases = {
        {54, 24},     // Basic case with two positive numbers
        {100, 0},     // Case with zero
        {-48, 18},    // Case with a negative number
        {17, 23},     // Case with two prime numbers
        {0, 0}        // Case where both numbers are zero
    };

    for (const auto& testCase : testCases) {
        int num1 = testCase.first;
        int num2 = testCase.second;
        int result = findGCD(num1, num2);
        std::cout << "The GCD of " << num1 << " and " << num2 << " is: " << result << std::endl;
    }

    return 0;
}