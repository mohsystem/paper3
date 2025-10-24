
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <limits>

// Function to convert a number to expanded form
// Security: Input validation ensures number is positive and within safe range
std::string expandedForm(long long num) {
    // Input validation: ensure number is positive (as per requirements)
    if (num <= 0) {
        throw std::invalid_argument("Number must be greater than 0");
    }
    
    // Input validation: prevent overflow in calculations
    if (num > LLONG_MAX / 10) {
        throw std::invalid_argument("Number too large");
    }
    
    // Convert number to string to safely iterate through digits
    // Using std::to_string is safe and prevents buffer overflows
    std::string numStr = std::to_string(num);
    std::vector<std::string> parts;
    
    // Reserve space to prevent multiple reallocations (performance and safety)
    parts.reserve(numStr.length());
    
    // Iterate through each digit with bounds checking
    for (size_t i = 0; i < numStr.length(); ++i) {
        // Bounds check: ensure we're within string length\n        if (i >= numStr.length()) {\n            break;\n        }\n        \n        char digit = numStr[i];\n        \n        // Validate that character is a digit (defense in depth)\n        if (digit < '0' || digit > '9') {\n            throw std::invalid_argument("Invalid digit in number");\n        }\n        \n        // Skip zeros as they don't appear in expanded form
        if (digit != '0') {
            // Calculate place value safely
            size_t zerosCount = numStr.length() - i - 1;
            
            // Prevent potential overflow in power calculation
            if (zerosCount > 18) { // 10^18 is near LLONG_MAX
                throw std::invalid_argument("Place value too large");
            }
            
            // Build the expanded part using safe string operations
            std::string part(1, digit);
            
            // Append zeros safely without pointer arithmetic
            for (size_t j = 0; j < zerosCount; ++j) {
                part += '0';
            }
            
            parts.push_back(part);
        }
    }
    
    // Handle edge case: if all digits were zero (shouldn't happen with num > 0)
    if (parts.empty()) {
        return "0";
    }
    
    // Join parts with " + " separator using safe string concatenation
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            result += " + ";
        }
        result += parts[i];
    }
    
    return result;
}

int main() {
    // Test cases with error handling
    try {
        std::cout << "Test 1: " << expandedForm(12) << std::endl;
        std::cout << "Expected: 10 + 2" << std::endl << std::endl;
        
        std::cout << "Test 2: " << expandedForm(42) << std::endl;
        std::cout << "Expected: 40 + 2" << std::endl << std::endl;
        
        std::cout << "Test 3: " << expandedForm(70304) << std::endl;
        std::cout << "Expected: 70000 + 300 + 4" << std::endl << std::endl;
        
        std::cout << "Test 4: " << expandedForm(9) << std::endl;
        std::cout << "Expected: 9" << std::endl << std::endl;
        
        std::cout << "Test 5: " << expandedForm(123456) << std::endl;
        std::cout << "Expected: 100000 + 20000 + 3000 + 400 + 50 + 6" << std::endl << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
