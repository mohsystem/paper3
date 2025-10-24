#include <iostream>

/**
 * @brief Calculates the factorial of a non-negative integer.
 * 
 * This function calculates n! = 1 * 2 * ... * n.
 * It handles non-negative inputs up to 20, as 21! exceeds the capacity
 * of a 64-bit unsigned long long.
 *
 * @param n The non-negative integer. Must be in the range [0, 20].
 * @return The factorial of n as an unsigned long long. Returns 0 if the
 *         input is negative or greater than 20, to indicate an error.
 */
unsigned long long factorial(int n) {
    // Rule #4: Input validation. Factorial is not defined for negative numbers.
    if (n < 0) {
        return 0; // Error case
    }

    // Rule #8: Validate input range to prevent integer overflow.
    // The largest factorial that fits in a 64-bit unsigned long long is 20!.
    if (n > 20) {
        return 0; // Error case: input too large, would cause overflow.
    }

    // Base case: 0! is defined as 1.
    if (n == 0) {
        return 1;
    }

    unsigned long long result = 1;
    for (int i = 1; i <= n; ++i) {
        // The check `n > 20` above ensures that this multiplication will not overflow.
        result *= i;
    }

    return result;
}

int main() {
    // A set of test cases, including valid, edge, and invalid inputs.
    const int test_cases[] = {0, 1, 5, 10, 20, -1, 21};
    const int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        int n = test_cases[i];
        unsigned long long result = factorial(n);

        std::cout << "Factorial of " << n << ": ";
        
        // A return value of 0 indicates an error for the invalid inputs n < 0 or n > 20.
        // For n=0, factorial is 1, so there is no ambiguity.
        if (result == 0 && (n < 0 || n > 20)) {
            std::cout << "Error: Input must be between 0 and 20." << std::endl;
        } else {
            std::cout << result << std::endl;
        }
    }

    return 0;
}