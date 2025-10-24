#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cctype>

/**
 * @brief Computes the largest product of a series of adjacent digits of a specified span.
 * 
 * @param digits The sequence of digits to analyze.
 * @param span The number of digits in each series.
 * @return The largest product found as a long long.
 * @throw std::invalid_argument if the span is invalid or the input string contains non-digit characters.
 */
long long largestProduct(const std::string& digits, int span) {
    if (span < 0) {
        throw std::invalid_argument("Span must be non-negative.");
    }
    if (static_cast<size_t>(span) > digits.length()) {
        throw std::invalid_argument("Span cannot be greater than the length of the digit string.");
    }
    for (char c : digits) {
        if (!isdigit(c)) {
            throw std::invalid_argument("Digit string must contain only digits.");
        }
    }

    if (span == 0) {
        return 1; // The product of an empty set is conventionally 1.
    }

    long long maxProduct = 0;
    for (size_t i = 0; i <= digits.length() - span; ++i) {
        long long currentProduct = 1;
        for (int j = 0; j < span; ++j) {
            currentProduct *= (digits[i + j] - '0');
        }
        if (currentProduct > maxProduct) {
            maxProduct = currentProduct;
        }
    }

    return maxProduct;
}

int main() {
    // Test Case 1
    std::cout << "Test Case 1: ('63915', 3)" << std::endl;
    std::cout << "Result: " << largestProduct("63915", 3) << std::endl; // Expected: 162
    std::cout << std::endl;

    // Test Case 2
    std::cout << "Test Case 2: ('123456789', 4)" << std::endl;
    std::cout << "Result: " << largestProduct("123456789", 4) << std::endl; // Expected: 3024
    std::cout << std::endl;
    
    // Test Case 3
    std::cout << "Test Case 3: ('1110987', 3)" << std::endl;
    std::cout << "Result: " << largestProduct("1110987", 3) << std::endl; // Expected: 504
    std::cout << std::endl;

    // Test Case 4
    std::cout << "Test Case 4: ('12345', 5)" << std::endl;
    std::cout << "Result: " << largestProduct("12345", 5) << std::endl; // Expected: 120
    std::cout << std::endl;

    // Test Case 5 (Invalid)
    std::cout << "Test Case 5: ('123', 4)" << std::endl;
    try {
        largestProduct("123", 4);
    } catch (const std::invalid_argument& e) {
        std::cout << "Result: " << e.what() << std::endl; // Expected: Exception
    }
    std::cout << std::endl;
    
    return 0;
}