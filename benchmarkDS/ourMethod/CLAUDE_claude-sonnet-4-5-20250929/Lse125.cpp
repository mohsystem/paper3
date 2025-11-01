
#include <iostream>
#include <string>
#include <cstring>

/**
 * Trims trailing whitespace from a string and returns the result.
 * Security measures:
 * - Bounds-safe string operations using std::string
 * - No pointer arithmetic on raw buffers
 * - Input validation
 * - Length checks to prevent resource exhaustion
 */
std::string trimTrailingWhitespace(const std::string& input) {
    // Handle empty string
    if (input.empty()) {
        return input;
    }
    
    // Find the last non-whitespace character
    // Start from the end and work backwards
    size_t lastNonWhitespace = input.length();
    while (lastNonWhitespace > 0 && std::isspace(static_cast<unsigned char>(input[lastNonWhitespace - 1]))) {
        lastNonWhitespace--;
    }
    
    // Return substring - std::string::substr is bounds-safe
    return input.substr(0, lastNonWhitespace);
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testCases[] = {
        "hello world   ",
        "no trailing",
        "   leading and trailing   ",
        "   ",
        ""
    };
    
    std::cout << "Test cases:" << std::endl;
    for (size_t i = 0; i < 5; i++) {
        // Validate test case pointer is not null
        if (testCases[i] != nullptr) {
            std::string result = trimTrailingWhitespace(std::string(testCases[i]));
            std::cout << "Test " << (i + 1) << ": [" << result << "]" << std::endl;
        }
    }
    
    // Process command line arguments if provided
    // Validate argc before accessing argv
    if (argc > 1) {
        std::cout << "\\nCommand line argument processing:" << std::endl;
        
        // Validate argv[1] is not null
        if (argv[1] != nullptr) {
            // Validate input length to prevent resource exhaustion
            const size_t MAX_INPUT_LENGTH = 10000;
            size_t inputLength = std::strlen(argv[1]);
            
            if (inputLength > MAX_INPUT_LENGTH) {
                std::cerr << "Error: Input exceeds maximum allowed length" << std::endl;
                return 1;
            }
            
            std::string input(argv[1]);
            std::string trimmed = trimTrailingWhitespace(input);
            std::cout << "[" << trimmed << "]" << std::endl;
        }
    }
    
    return 0;
}
