
#include <iostream>
#include <string>
#include <stdexcept>
#include <climits>
#include <cstdint>

// Function to find the largest product of adjacent digits with specified span
// Returns -1 on error, largest product otherwise
int64_t largestProduct(const std::string& input, int span) {
    // Security: Validate span is non-negative to prevent negative indexing
    if (span < 0) {
        throw std::invalid_argument("Span must be non-negative");
    }
    
    // Security: Handle edge case where span is 0 (product is 1 by convention)
    if (span == 0) {
        return 1;
    }
    
    // Security: Validate input length to prevent buffer overruns
    if (input.empty()) {
        throw std::invalid_argument("Input string cannot be empty");
    }
    
    // Security: Check if span exceeds input length
    if (static_cast<size_t>(span) > input.length()) {
        throw std::invalid_argument("Span cannot be larger than input length");
    }
    
    // Security: Validate that input contains only digits
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] < '0' || input[i] > '9') {
            throw std::invalid_argument("Input must contain only digits");
        }
    }
    
    int64_t maxProduct = 0;
    
    // Security: Use size_t for loop counter to match string::length() type
    // Iterate through all possible series positions
    for (size_t i = 0; i <= input.length() - static_cast<size_t>(span); ++i) {
        int64_t product = 1;
        
        // Security: Calculate product with overflow check
        for (int j = 0; j < span; ++j) {
            // Security: Bounds check before accessing input[i + j]
            int digit = input[i + static_cast<size_t>(j)] - '0';
            
            // Security: Check for potential overflow before multiplication
            if (product > INT64_MAX / 10) {
                throw std::overflow_error("Product calculation would overflow");
            }
            
            product *= digit;
        }
        
        // Track maximum product
        if (product > maxProduct) {
            maxProduct = product;
        }
    }
    
    return maxProduct;
}

int main() {
    // Test case 1: Example from problem
    try {
        std::string input1 = "63915";
        int span1 = 3;
        int64_t result1 = largestProduct(input1, span1);
        std::cout << "Test 1: input=\\"" << input1 << "\\", span=" << span1 
                  << " -> " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 error: " << e.what() << std::endl;
    }
    
    // Test case 2: Span of 0 (should return 1)
    try {
        std::string input2 = "123";
        int span2 = 0;
        int64_t result2 = largestProduct(input2, span2);
        std::cout << "Test 2: input=\\"" << input2 << "\\", span=" << span2 
                  << " -> " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 error: " << e.what() << std::endl;
    }
    
    // Test case 3: Single digit span
    try {
        std::string input3 = "987654321";
        int span3 = 1;
        int64_t result3 = largestProduct(input3, span3);
        std::cout << "Test 3: input=\\"" << input3 << "\\", span=" << span3 
                  << " -> " << result3 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 error: " << e.what() << std::endl;
    }
    
    // Test case 4: Span equals input length
    try {
        std::string input4 = "12345";
        int span4 = 5;
        int64_t result4 = largestProduct(input4, span4);
        std::cout << "Test 4: input=\\"" << input4 << "\\", span=" << span4 
                  << " -> " << result4 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 error: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid input - span larger than input
    try {
        std::string input5 = "123";
        int span5 = 5;
        int64_t result5 = largestProduct(input5, span5);
        std::cout << "Test 5: input=\\"" << input5 << "\\", span=" << span5 
                  << " -> " << result5 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 error: " << e.what() << std::endl;
    }
    
    return 0;
}
