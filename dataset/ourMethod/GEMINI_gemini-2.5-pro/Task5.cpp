#include <iostream>

/**
 * @brief Calculates the multiplicative persistence of a positive integer.
 *
 * Multiplicative persistence is the number of times you must multiply the digits
 * in a number until you reach a single digit.
 *
 * @param n The positive integer. Must be non-negative.
 * @return The multiplicative persistence of n.
 */
int persistence(long long n) {
    // A single-digit number (0-9) has a persistence of 0.
    // The prompt specifies a positive parameter, but this also correctly handles 0.
    if (n >= 0 && n < 10) {
        return 0;
    }
    // We can assume n is positive based on the prompt, so no need for n < 0 check.

    int count = 0;
    long long current_num = n;

    while (current_num >= 10) {
        count++;
        long long product = 1;
        long long temp_num = current_num;
        
        if (temp_num == 0) { // Should not happen if current_num >= 10
            product = 0;
        }

        while (temp_num > 0) {
            product *= (temp_num % 10);
            temp_num /= 10;
        }
        current_num = product;
    }

    return count;
}

int main() {
    // Test cases
    long long test_cases[] = {39, 999, 4, 277777788888899LL, 679};
    int expected_results[] = {3, 4, 0, 11, 5};
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        long long num = test_cases[i];
        int result = persistence(num);
        std::cout << "persistence(" << num << ") -> " << result
                  << " (Expected: " << expected_results[i] << ")" << std::endl;
    }

    return 0;
}