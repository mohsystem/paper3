#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cctype>

/**
 * Computes the largest product of a series of adjacent digits.
 *
 * @param input The sequence of digits to analyze.
 * @param span The number of digits in each series.
 * @return The largest product.
 * @throw std::invalid_argument if the input is invalid.
 */
long long largestProduct(const std::string& input, int span) {
    if (span < 0) {
        throw std::invalid_argument("Span cannot be negative.");
    }
    if (static_cast<size_t>(span) > input.length()) {
        throw std::invalid_argument("Span cannot be larger than the input string length.");
    }
    if (!std::all_of(input.begin(), input.end(), ::isdigit)) {
        throw std::invalid_argument("Input string must contain only digits.");
    }

    if (span == 0) {
        return 1;
    }

    long long maxProduct = 0;

    for (size_t i = 0; i <= input.length() - span; ++i) {
        long long currentProduct = 1;
        for (int j = 0; j < span; ++j) {
            currentProduct *= (input[i + j] - '0');
        }
        if (currentProduct > maxProduct) {
            maxProduct = currentProduct;
        }
    }

    return maxProduct;
}

void run_test(const std::string& input, int span) {
    std::cout << "Input: \"" << (input.length() > 10 ? input.substr(0, 10) + "..." : input) 
              << "\", Span: " << span;
    try {
        long long result = largestProduct(input, span);
        std::cout << ", Largest Product: " << result << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << ", Error: " << e.what() << std::endl;
    }
}

int main() {
    // Test Case 1: Example from prompt
    run_test("63915", 3); // Expected: 162

    // Test Case 2: Contains a zero
    run_test("1027839564", 5); // Expected: 22680 (3*9*5*6*4)

    // Test Case 3: A longer number
    run_test("73167176531330624919225119674426574742355349194934", 6); // Expected: 23520

    // Test Case 4: Span is 0
    run_test("123", 0); // Expected: 1

    // Test Case 5: Span is 1
    run_test("18395", 1); // Expected: 9

    // Test Case 6: Invalid input (demonstrating exception handling)
    run_test("123a45", 3);

    return 0;
}