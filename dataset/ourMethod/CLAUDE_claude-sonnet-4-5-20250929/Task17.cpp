
#include <iostream>
#include <string>
#include <cctype>
#include <cstring>

// Function to convert string to Jaden Case
// Each word's first letter is capitalized\n// Security: Input validation, bounds checking, no dynamic allocation from untrusted size\nstd::string toJadenCase(const std::string& input) {\n    // Validate input: check for empty string\n    if (input.empty()) {\n        return input;\n    }\n    \n    // Validate input length to prevent excessive memory usage\n    // Maximum reasonable tweet length is 280 characters, allow some buffer\n    const size_t MAX_INPUT_LENGTH = 10000;\n    if (input.length() > MAX_INPUT_LENGTH) {\n        throw std::invalid_argument("Input string exceeds maximum length");\n    }\n    \n    // Create result string - safe as we know input size\n    std::string result = input;\n    \n    // Flag to track if we're at the start of a word
    bool newWord = true;
    
    // Iterate through each character with bounds checking
    for (size_t i = 0; i < result.length(); ++i) {
        // Bounds check (redundant but defensive)
        if (i >= result.length()) {
            break;
        }
        
        if (newWord && std::isalpha(static_cast<unsigned char>(result[i]))) {
            // Capitalize first letter of word
            // Use unsigned char cast to avoid undefined behavior with negative char values
            result[i] = std::toupper(static_cast<unsigned char>(result[i]));
            newWord = false;
        } else if (std::isspace(static_cast<unsigned char>(result[i]))) {
            // Space indicates start of new word
            newWord = true;
        }
    }
    
    return result;
}

int main() {
    // Test case 1: Normal sentence
    std::string test1 = "How can mirrors be real if our eyes aren't real";
    std::cout << "Test 1: " << toJadenCase(test1) << std::endl;
    
    // Test case 2: Empty string
    std::string test2 = "";
    std::cout << "Test 2: \\"" << toJadenCase(test2) << "\\"" << std::endl;
    
    // Test case 3: Single word
    std::string test3 = "hello";
    std::cout << "Test 3: " << toJadenCase(test3) << std::endl;
    
    // Test case 4: Multiple spaces
    std::string test4 = "this  has   multiple spaces";
    std::cout << "Test 4: " << toJadenCase(test4) << std::endl;
    
    // Test case 5: Mixed case input
    std::string test5 = "tHiS iS MiXeD cAsE";
    std::cout << "Test 5: " << toJadenCase(test5) << std::endl;
    
    return 0;
}
