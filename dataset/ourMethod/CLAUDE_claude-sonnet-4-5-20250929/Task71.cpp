
#include <iostream>
#include <string>
#include <limits>
#include <cerrno>
#include <cstdlib>

// Convert string to integer with validation
// Returns true on success, false on failure
bool stringToInt(const std::string& input, int& result) {
    // Rule #3: Validate input is not empty
    if (input.empty()) {
        std::cerr << "Error: Empty input string" << std::endl;
        return false;
    }
    
    // Rule #3: Check for reasonable length to prevent resource exhaustion
    if (input.length() > 20) {
        std::cerr << "Error: Input string too long" << std::endl;
        return false;
    }
    
    // Rule #3: Validate input contains only valid integer characters
    size_t start = 0;
    if (input[0] == '-' || input[0] == '+') {
        start = 1;
    }
    
    if (start >= input.length()) {
        std::cerr << "Error: Invalid input format" << std::endl;
        return false;
    }
    
    for (size_t i = start; i < input.length(); ++i) {
        if (input[i] < '0' || input[i] > '9') {
            std::cerr << "Error: Invalid character in input" << std::endl;
            return false;
        }
    }
    
    // Use strtol for safe conversion with error checking
    errno = 0;
    char* endptr = nullptr;
    long value = std::strtol(input.c_str(), &endptr, 10);
    
    // Rule #3: Check for conversion errors
    if (errno == ERANGE || value > std::numeric_limits<int>::max() || 
        value < std::numeric_limits<int>::min()) {
        std::cerr << "Error: Number out of range for int" << std::endl;
        return false;
    }
    
    // Ensure entire string was consumed
    if (endptr != input.c_str() + input.length()) {
        std::cerr << "Error: Invalid trailing characters" << std::endl;
        return false;
    }
    
    result = static_cast<int>(value);
    return true;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::string testCases[] = {
        "42",
        "-123",
        "2147483647",
        "-2147483648",
        "invalid123"
    };
    
    std::cout << "=== Test Cases ===" << std::endl;
    for (int i = 0; i < 5; ++i) {
        int result = 0;
        std::cout << "Input: \\"" << testCases[i] << "\\" -> ";
        if (stringToInt(testCases[i], result)) {
            std::cout << "Success: " << result << std::endl;
        } else {
            std::cout << "Failed" << std::endl;
        }
    }
    
    // Rule #1: Validate command line arguments
    if (argc < 2) {
        std::cerr << "\\n=== Command Line Argument ===" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <integer_string>" << std::endl;
        return 1;
    }
    
    std::cout << "\\n=== Command Line Conversion ===" << std::endl;
    int result = 0;
    std::string input(argv[1]);
    
    if (stringToInt(input, result)) {
        std::cout << "Successfully converted \\"" << input << "\\" to: " << result << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to convert \\"" << input << "\\"" << std::endl;
        return 1;
    }
}
