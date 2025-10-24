#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>
#include <algorithm>

/**
 * Computes the largest product of a series of adjacent digits.
 *
 * @param input The sequence of digits as a std::string.
 * @param span The number of digits in each series.
 * @return The largest product found (as a 64-bit integer).
 * @throws std::invalid_argument if the input is invalid.
 */
long long largestProduct(const std::string& input, int span) {
    // 1. Input Validation
    if (span < 0) {
        throw std::invalid_argument("Span cannot be negative.");
    }
    for (char c : input) {
        if (!isdigit(c)) {
            throw std::invalid_argument("Input string must contain only digits.");
        }
    }
    if (static_cast<size_t>(span) > input.length()) {
        throw std::invalid_argument("Span cannot be greater than the input string length.");
    }

    // 2. Edge Case
    if (span == 0) {
        return 1;
    }

    // 3. Calculation
    long long maxProduct = 0;
    for (size_t i = 0; i <= input.length() - span; ++i) {
        long long currentProduct = 1;
        for (int j = 0; j < span; ++j) {
            // Convert character to its numeric value and multiply
            currentProduct *= (input[i + j] - '0');
        }
        if (currentProduct > maxProduct) {
            maxProduct = currentProduct;
        }
    }
    return maxProduct;
}

int main() {
    // 5 Test Cases
    try {
        // Test Case 1: Example from prompt
        std::cout << largestProduct("63915", 3) << std::endl;

        // Test Case 2: A sequence containing zero
        std::cout << largestProduct("1234560789", 5) << std::endl;

        // Test Case 3: Span of 1
        std::cout << largestProduct("987654321", 1) << std::endl;

        // Test Case 4: Span equals the length of the input
        std::cout << largestProduct("12345", 5) << std::endl;

        // Test Case 5: Span of 0
        std::cout << largestProduct("111111", 0) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}