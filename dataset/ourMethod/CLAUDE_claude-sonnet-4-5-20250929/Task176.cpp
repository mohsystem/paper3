
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstdint>

class ProductOfNumbers {
private:
    std::vector<int> prefixProducts;
    
public:
    // Initialize with empty stream - starts with prefix product of 1
    ProductOfNumbers() {
        // Validate initial state - start with 1 as the identity element for multiplication
        prefixProducts.push_back(1);
    }
    
    // Add number to stream with input validation
    void add(int num) {
        // Validate input range according to constraints (0 <= num <= 100)
        if (num < 0 || num > 100) {
            throw std::invalid_argument("Input must be between 0 and 100");
        }
        
        // When zero is added, reset the prefix product array to avoid multiplying by zero
        // This handles the zero case efficiently - any product including this zero will be zero
        if (num == 0) {
            prefixProducts.clear();
            prefixProducts.push_back(1);
        } else {
            // Validate that multiplication won't cause undefined behavior\n            // Check for potential overflow before multiplication (though problem guarantees no overflow)\n            int64_t lastProduct = static_cast<int64_t>(prefixProducts.back());\n            int64_t newProduct = lastProduct * static_cast<int64_t>(num);\n            \n            // Additional safety check for 32-bit integer bounds\n            if (newProduct > INT32_MAX || newProduct < INT32_MIN) {\n                throw std::overflow_error("Product exceeds 32-bit integer range");\n            }\n            \n            // Safe to add the new prefix product\n            prefixProducts.push_back(static_cast<int>(newProduct));\n        }\n    }\n    \n    // Get product of last k numbers with bounds validation\n    int getProduct(int k) {\n        // Validate k is within valid range (1 <= k <= 40000)\n        if (k < 1 || k > 40000) {\n            throw std::invalid_argument("k must be between 1 and 40000");\n        }\n        \n        // Validate that we have at least k numbers in the stream\n        // prefixProducts.size() - 1 gives us the count of actual numbers added\n        size_t numCount = prefixProducts.size() - 1;\n        if (static_cast<size_t>(k) > numCount) {\n            throw std::invalid_argument("Not enough numbers in stream");\n        }\n        \n        // If k is larger than available non-zero sequence, product must be zero\n        // This happens when a zero was recently added and k extends before it\n        if (static_cast<size_t>(k) >= prefixProducts.size()) {\n            return 0;\n        }\n        \n        // Calculate product using prefix products: product(last k) = prefixProducts[n] / prefixProducts[n-k]\n        // Bounds check for array access\n        size_t endIdx = prefixProducts.size() - 1;\n        size_t startIdx = endIdx - static_cast<size_t>(k);\n        \n        // Additional validation that indices are within bounds\n        if (startIdx >= prefixProducts.size() || endIdx >= prefixProducts.size()) {\n            throw std::out_of_range("Index calculation error");\n        }\n        \n        // Safe division since we're using integer division and startIdx product is always a divisor
        int result = prefixProducts[endIdx] / prefixProducts[startIdx];
        return result;
    }
};

// Test cases
int main() {
    try {
        // Test case 1: Example from problem
        ProductOfNumbers productOfNumbers1;
        productOfNumbers1.add(3);
        productOfNumbers1.add(0);
        productOfNumbers1.add(2);
        productOfNumbers1.add(5);
        productOfNumbers1.add(4);
        std::cout << "Test 1 - getProduct(2): " << productOfNumbers1.getProduct(2) << " (expected 20)" << std::endl;
        std::cout << "Test 1 - getProduct(3): " << productOfNumbers1.getProduct(3) << " (expected 40)" << std::endl;
        std::cout << "Test 1 - getProduct(4): " << productOfNumbers1.getProduct(4) << " (expected 0)" << std::endl;
        productOfNumbers1.add(8);
        std::cout << "Test 1 - getProduct(2): " << productOfNumbers1.getProduct(2) << " (expected 32)" << std::endl;
        
        // Test case 2: All non-zero numbers
        ProductOfNumbers productOfNumbers2;
        productOfNumbers2.add(2);
        productOfNumbers2.add(3);
        productOfNumbers2.add(4);
        std::cout << "Test 2 - getProduct(3): " << productOfNumbers2.getProduct(3) << " (expected 24)" << std::endl;
        
        // Test case 3: Single element
        ProductOfNumbers productOfNumbers3;
        productOfNumbers3.add(5);
        std::cout << "Test 3 - getProduct(1): " << productOfNumbers3.getProduct(1) << " (expected 5)" << std::endl;
        
        // Test case 4: Multiple zeros
        ProductOfNumbers productOfNumbers4;
        productOfNumbers4.add(1);
        productOfNumbers4.add(0);
        productOfNumbers4.add(2);
        productOfNumbers4.add(0);
        productOfNumbers4.add(3);
        std::cout << "Test 4 - getProduct(1): " << productOfNumbers4.getProduct(1) << " (expected 3)" << std::endl;
        
        // Test case 5: Large sequence
        ProductOfNumbers productOfNumbers5;
        productOfNumbers5.add(1);
        productOfNumbers5.add(2);
        productOfNumbers5.add(3);
        productOfNumbers5.add(4);
        productOfNumbers5.add(5);
        std::cout << "Test 5 - getProduct(3): " << productOfNumbers5.getProduct(3) << " (expected 60)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
