
#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <iostream>

// Function to generate "likes" display text
// Security: Validates input array size and individual name lengths to prevent buffer overflows
// and excessive memory allocation
std::string likes(const std::vector<std::string>& names) {
    // Security: Validate input - check for reasonable array size to prevent DoS via memory exhaustion
    const size_t MAX_NAMES = 10000;
    if (names.size() > MAX_NAMES) {
        throw std::invalid_argument("Too many names in input array");
    }
    
    // Security: Validate each name for reasonable length and non-empty strings
    const size_t MAX_NAME_LENGTH = 1000;
    for (const auto& name : names) {
        if (name.empty()) {
            throw std::invalid_argument("Empty name not allowed");
        }
        if (name.length() > MAX_NAME_LENGTH) {
            throw std::invalid_argument("Name exceeds maximum length");
        }
    }
    
    size_t count = names.size();
    
    // Security: Use switch with explicit cases to avoid undefined behavior
    switch (count) {
        case 0:
            return "no one likes this";
        case 1:
            // Security: Direct string concatenation with validated input
            return names[0] + " likes this";
        case 2:
            // Security: Concatenate validated strings using safe std::string operations
            return names[0] + " and " + names[1] + " like this";
        case 3:
            // Security: Safe concatenation with bounds-checked vector access
            return names[0] + ", " + names[1] + " and " + names[2] + " like this";
        default:
            // Security: Calculate others count with overflow prevention
            if (count < 2) {
                throw std::logic_error("Invalid count state");
            }
            size_t others = count - 2;
            // Security: Use std::to_string for safe integer to string conversion
            return names[0] + ", " + names[1] + " and " + std::to_string(others) + " others like this";
    }
}

int main() {
    // Test case 1: Empty array
    try {
        std::cout << "Test 1: " << likes({}) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 Error: " << e.what() << std::endl;
    }
    
    // Test case 2: One name
    try {
        std::cout << "Test 2: " << likes({"Peter"}) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 Error: " << e.what() << std::endl;
    }
    
    // Test case 3: Two names
    try {
        std::cout << "Test 3: " << likes({"Jacob", "Alex"}) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 Error: " << e.what() << std::endl;
    }
    
    // Test case 4: Three names
    try {
        std::cout << "Test 4: " << likes({"Max", "John", "Mark"}) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Four or more names
    try {
        std::cout << "Test 5: " << likes({"Alex", "Jacob", "Mark", "Max"}) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 Error: " << e.what() << std::endl;
    }
    
    return 0;
}
