
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <limits>

// Convert binary array to integer
// Validates input to prevent overflow and ensures only 0 or 1 values
int binaryArrayToInt(const std::vector<int>& arr) {
    // Input validation: check for null/empty array
    if (arr.empty()) {
        throw std::invalid_argument("Array cannot be empty");
    }
    
    // Security: prevent overflow by limiting array size
    // Maximum 31 bits for positive int (avoid sign bit issues)
    if (arr.size() > 31) {
        throw std::invalid_argument("Array too large - would cause integer overflow");
    }
    
    int result = 0;
    
    // Process each bit with validation
    for (size_t i = 0; i < arr.size(); ++i) {
        // Security: validate each element is only 0 or 1
        if (arr[i] != 0 && arr[i] != 1) {
            throw std::invalid_argument("Array must contain only 0 or 1");
        }
        
        // Security: check for overflow before shift operation
        // If result would overflow when shifted, throw error
        if (result > (std::numeric_limits<int>::max() >> 1)) {
            throw std::overflow_error("Integer overflow detected");
        }
        
        // Shift left and add current bit
        result = (result << 1) | arr[i];
    }
    
    return result;
}

int main() {
    // Test case 1: [0, 0, 0, 1] => 1
    try {
        std::vector<int> test1 = {0, 0, 0, 1};
        std::cout << "Test 1: " << binaryArrayToInt(test1) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 error: " << e.what() << std::endl;
    }
    
    // Test case 2: [0, 0, 1, 0] => 2
    try {
        std::vector<int> test2 = {0, 0, 1, 0};
        std::cout << "Test 2: " << binaryArrayToInt(test2) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 error: " << e.what() << std::endl;
    }
    
    // Test case 3: [0, 1, 0, 1] => 5
    try {
        std::vector<int> test3 = {0, 1, 0, 1};
        std::cout << "Test 3: " << binaryArrayToInt(test3) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 error: " << e.what() << std::endl;
    }
    
    // Test case 4: [1, 0, 0, 1] => 9
    try {
        std::vector<int> test4 = {1, 0, 0, 1};
        std::cout << "Test 4: " << binaryArrayToInt(test4) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 error: " << e.what() << std::endl;
    }
    
    // Test case 5: [1, 1, 1, 1] => 15
    try {
        std::vector<int> test5 = {1, 1, 1, 1};
        std::cout << "Test 5: " << binaryArrayToInt(test5) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 error: " << e.what() << std::endl;
    }
    
    return 0;
}
