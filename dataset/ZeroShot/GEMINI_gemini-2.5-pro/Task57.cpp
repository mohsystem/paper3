#include <iostream>
#include <stdexcept>

/**
 * Calculates the factorial of a non-negative integer securely.
 * Uses unsigned long long, which can hold up to 20!
 *
 * @param n The non-negative integer.
 * @return The factorial of n as an unsigned long long.
 * @throws std::invalid_argument if n is negative.
 * @throws std::overflow_error if n > 20, as it would cause an overflow.
 */
unsigned long long calculateFactorial(int n) {
    // 1. Input Validation: Factorial is not defined for negative numbers.
    if (n < 0) {
        throw std::invalid_argument("Input must be a non-negative number.");
    }
    
    // 2. Input Validation: Prevent overflow. 20! is the max for unsigned long long.
    if (n > 20) {
        throw std::overflow_error("Input n > 20 causes overflow for unsigned long long.");
    }

    // 3. Use an iterative approach to avoid stack overflow.
    unsigned long long result = 1ULL;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    // 5 test cases
    int testCases[] = {0, 1, 5, 20, -5};
    
    std::cout << "CPP Factorial Test Cases:" << std::endl;
    for (int test : testCases) {
        try {
            unsigned long long factorial = calculateFactorial(test);
            std::cout << "Factorial of " << test << " is " << factorial << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error for input " << test << ": " << e.what() << std::endl;
        }
    }
    return 0;
}