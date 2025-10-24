#include <iostream>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <vector>

struct Product {
    std::string name;
    double price;
};

// Global immutable product table
const std::unordered_map<std::string, Product> PRODUCT_TABLE = {
    {"A123", {"Laptop", 999.99}},
    {"B456", {"Mouse", 25.50}},
    {"C789", {"Keyboard", 75.00}}
};

/**
 * @brief Inquires the product table for product details.
 * 
 * @param productId The ID of the product to look for.
 * @return A string with product details or an error message.
 */
std::string getProductDetails(const std::string& productId) {
    // Rule #3: Validate input. Treat all inputs as untrusted.
    if (productId.empty()) {
        return "Error: Product ID cannot be empty.";
    }

    // Rule #1: Use safe parsing and formatting (std::stringstream)
    auto it = PRODUCT_TABLE.find(productId);
    
    std::stringstream ss;
    if (it != PRODUCT_TABLE.end()) {
        const Product& product = it->second;
        ss << "Product: " << product.name << ", Price: $" 
           << std::fixed << std::setprecision(2) << product.price;
    } else {
        // Sanitize output by limiting length
        std::string sanitizedId = productId;
        if (sanitizedId.length() > 50) {
            sanitizedId = sanitizedId.substr(0, 50);
        }
        ss << "Product with ID '" << sanitizedId << "' not found.";
    }
    
    return ss.str();
}

int main() {
    std::cout << "--- Product Inquiry System ---" << std::endl;

    std::vector<std::string> testCases = {
        "A123",       // Test Case 1: Valid, existing product ID
        "C789",       // Test Case 2: Another valid, existing product ID
        "X999",       // Test Case 3: Valid format, but non-existent ID
        ""            // Test Case 4: Invalid empty string
    };
    
    // C++ doesn't have a direct equivalent of 'null' for std::string reference,
    // so we test the empty case. We'll add a nullptr test for C-style strings if needed.

    int testNum = 1;
    for (const auto& test_input : testCases) {
        std::cout << "Test Case " << testNum++ << " (Input: \"" << test_input << "\"):" << std::endl;
        std::string result = getProductDetails(test_input);
        std::cout << "  Result: " << result << std::endl << std::endl;
    }

    // Test case 5: A conceptual equivalent of a "null" or invalid input in some contexts
    std::cout << "Test Case 5 (Input: conceptually invalid/uninitialized):" << std::endl;
    std::string uninitialized_string; // Default-constructs to empty
    std::string result_uninitialized = getProductDetails(uninitialized_string);
    std::cout << "  Result: " << result_uninitialized << std::endl << std::endl;

    return 0;
}