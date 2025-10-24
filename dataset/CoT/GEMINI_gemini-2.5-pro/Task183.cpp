#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cctype>

/**
 * Computes the largest product of a series of adjacent digits of a specified span.
 *
 * @param input The sequence of digits to analyze.
 * @param span  The number of digits in each series.
 * @return The largest product.
 * @throws std::invalid_argument if the input is invalid.
 */
long long largestProduct(const std::string& input, int span) {
    // 1. Input Validation
    if (span < 0) {
        throw std::invalid_argument("Span must not be negative.");
    }
    if (static_cast<size_t>(span) > input.length()) {
        throw std::invalid_argument("Span must be smaller than or equal to the length of the input string.");
    }
    for (char c : input) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            throw std::invalid_argument("Input string must only contain digits.");
        }
    }

    // 2. Handle edge case: span is 0, the product of an empty set is 1.
    if (span == 0) {
        return 1;
    }

    // 3. Main Algorithm
    long long maxProduct = 0;

    for (size_t i = 0; i <= input.length() - span; ++i) {
        long long currentProduct = 1;
        for (int j = 0; j < span; ++j) {
            currentProduct *= (input[i + j] - '0');
        }
        maxProduct = std::max(maxProduct, currentProduct);
    }

    return maxProduct;
}

int main() {
    // Test Cases
    std::vector<std::string> inputs = {
        "63915",
        "1027839564",
        "12345",
        "12345",
        "73167176531330624919225119674426574742355349194934"
    };
    std::vector<int> spans = {3, 4, 5, 0, 6};
    std::vector<long long> expectedOutputs = {162, 5832, 120, 1, 23520};

    for (size_t i = 0; i < inputs.size(); ++i) {
        try {
            long long result = largestProduct(inputs[i], spans[i]);
            std::cout << "Test Case " << (i + 1) << ":" << std::endl;
            std::cout << "Input: \"" << inputs[i] << "\", Span: " << spans[i] << std::endl;
            std::cout << "Result: " << result << std::endl;
            std::cout << "Expected: " << expectedOutputs[i] << std::endl;
            std::cout << "Status: " << (result == expectedOutputs[i] ? "Passed" : "Failed") << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Test Case " << (i + 1) << " threw an exception: " << e.what() << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }

    // Test invalid cases
    try {
        largestProduct("123", 4);
    } catch (const std::invalid_argument& e) {
        std::cout << "Test Case 6 (Invalid Span): Passed - " << e.what() << std::endl;
    }
    try {
        largestProduct("1a2", 2);
    } catch (const std::invalid_argument& e) {
        std::cout << "Test Case 7 (Invalid Chars): Passed - " << e.what() << std::endl;
    }

    return 0;
}