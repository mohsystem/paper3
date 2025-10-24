#include <iostream>
#include <stdexcept>

/**
 * Calculates the factorial of a non-negative integer.
 *
 * @param n The non-negative integer. Input is limited to 20
 *          to prevent long long overflow.
 * @return The factorial of n as a long long.
 * @throws std::invalid_argument if n is negative.
 * @throws std::overflow_error if n > 20, which would overflow a long long.
 */
long long factorial(int n) {
    if (n < 0) {
        throw std::invalid_argument("Factorial is not defined for negative numbers.");
    }
    if (n > 20) {
        // 21! overflows a 64-bit long long
        throw std::overflow_error("Input " + std::to_string(n) + " is too large, it will cause a long long overflow.");
    }
    if (n == 0) {
        return 1LL;
    }

    long long result = 1LL;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    int testCases[] = {0, 1, 5, 12, 20, -1, 21};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int testCase = testCases[i];
        try {
            std::cout << "Factorial of " << testCase << " is: " << factorial(testCase) << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error calculating factorial for " << testCase << ": " << e.what() << std::endl;
        }
    }

    return 0;
}