
#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>
#include <cstdlib>
#include <cerrno>
#include <climits>

const int OFFSET = 1000;
// Prevent integer overflow by setting safe bounds
const long long MAX_INPUT_VALUE = LLONG_MAX - OFFSET;
const long long MIN_INPUT_VALUE = LLONG_MIN - OFFSET;

// Adds an offset of 1000 to the input value
long long addOffset(const std::string& input) {
    // Input validation: check for empty input
    if (input.empty()) {
        throw std::invalid_argument("Input cannot be empty");
    }
    
    // Sanitize input: trim whitespace
    std::string sanitized = input;
    sanitized.erase(0, sanitized.find_first_not_of(" \\t\\n\\r"));
    sanitized.erase(sanitized.find_last_not_of(" \\t\\n\\r") + 1);
    
    if (sanitized.empty()) {
        throw std::invalid_argument("Input cannot be empty");
    }
    
    // Parse input with error checking
    char* end = nullptr;
    errno = 0;
    long long value = std::strtoll(sanitized.c_str(), &end, 10);
    
    // Check for conversion errors
    if (errno == ERANGE) {
        throw std::overflow_error("Input value out of range");
    }
    if (end == sanitized.c_str() || *end != '\\0') {
        throw std::invalid_argument("Invalid numeric input");
    }
    
    // Integer overflow/underflow check before addition
    if (value > MAX_INPUT_VALUE) {
        throw std::overflow_error("Input value too large, would cause overflow");
    }
    if (value < MIN_INPUT_VALUE) {
        throw std::underflow_error("Input value too small, would cause underflow");
    }
    
    // Safe addition after validation
    return value + OFFSET;
}

int main() {
    // Test case 1: Normal positive value
    try {
        std::string input1 = "500";
        long long result1 = addOffset(input1);
        std::cout << "Input: " << input1 << ", Result: " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 2: Negative value
    try {
        std::string input2 = "-200";
        long long result2 = addOffset(input2);
        std::cout << "Input: " << input2 << ", Result: " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 3: Zero
    try {
        std::string input3 = "0";
        long long result3 = addOffset(input3);
        std::cout << "Input: " << input3 << ", Result: " << result3 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 4: Large value near max (should prevent overflow)
    try {
        std::string input4 = "9223372036854774807";
        long long result4 = addOffset(input4);
        std::cout << "Input: " << input4 << ", Result: " << result4 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid input
    try {
        std::string input5 = "invalid";
        long long result5 = addOffset(input5);
        std::cout << "Input: " << input5 << ", Result: " << result5 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
