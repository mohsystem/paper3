
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <cctype>
#include <algorithm>

// Validates that input string length is within acceptable bounds
bool validateStringLength(const std::string& input, size_t minLen, size_t maxLen) {
    return input.length() >= minLen && input.length() <= maxLen;
}

// Validates that string contains only alphanumeric characters and spaces
bool validateAlphanumeric(const std::string& input) {
    if (input.empty()) return false;
    
    for (char c : input) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != ' ') {
            return false;
        }
    }
    return true;
}

// Validates integer input within specified range
bool validateInteger(int value, int minVal, int maxVal) {
    return value >= minVal && value <= maxVal;
}

// Safe integer input function with validation
bool getValidatedInteger(int& output, int minVal, int maxVal) {
    std::string input;
    
    if (!std::getline(std::cin, input)) {
        return false;
    }
    
    // Validate length to prevent overflow attempts
    if (input.length() > 10) {
        return false;
    }
    
    // Check for empty input
    if (input.empty()) {
        return false;
    }
    
    // Validate that string contains only digits and optional leading minus
    size_t start = 0;
    if (input[0] == '-') {
        if (input.length() == 1) return false;
        start = 1;
    }
    
    for (size_t i = start; i < input.length(); i++) {
        if (!std::isdigit(static_cast<unsigned char>(input[i]))) {
            return false;
        }
    }
    
    try {
        // Use stoi with error checking
        size_t pos = 0;
        int value = std::stoi(input, &pos);
        
        // Ensure entire string was consumed
        if (pos != input.length()) {
            return false;
        }
        
        // Validate range
        if (!validateInteger(value, minVal, maxVal)) {
            return false;
        }
        
        output = value;
        return true;
    } catch (...) {
        return false;
    }
}

// Safe string input function with validation
bool getValidatedString(std::string& output, size_t minLen, size_t maxLen) {
    std::string input;
    
    if (!std::getline(std::cin, input)) {
        return false;
    }
    
    // Validate length
    if (!validateStringLength(input, minLen, maxLen)) {
        return false;
    }
    
    // Validate content - only alphanumeric and spaces
    if (!validateAlphanumeric(input)) {
        return false;
    }
    
    output = input;
    return true;
}

// Performs operations on validated input
std::string processInput(const std::string& text, int operation) {
    std::string result = text;
    
    switch (operation) {
        case 1: // Convert to uppercase
            std::transform(result.begin(), result.end(), result.begin(),
                         [](unsigned char c) { return std::toupper(c); });
            break;
        case 2: // Convert to lowercase
            std::transform(result.begin(), result.end(), result.begin(),
                         [](unsigned char c) { return std::tolower(c); });
            break;
        case 3: // Count characters
            return "Character count: " + std::to_string(result.length());
        default:
            return "Invalid operation";
    }
    
    return result;
}

int main() {
    std::cout << "=== Secure Input Processing Program ===" << std::endl;
    
    // Test case 1: Valid uppercase conversion
    std::cout << "\\nTest 1: Uppercase conversion" << std::endl;
    {
        std::string text = "Hello World 123";
        int op = 1;
        if (validateAlphanumeric(text) && validateStringLength(text, 1, 100)) {
            std::cout << "Input: " << text << std::endl;
            std::cout << "Output: " << processInput(text, op) << std::endl;
        }
    }
    
    // Test case 2: Valid lowercase conversion
    std::cout << "\\nTest 2: Lowercase conversion" << std::endl;
    {
        std::string text = "TESTING ABC 456";
        int op = 2;
        if (validateAlphanumeric(text) && validateStringLength(text, 1, 100)) {
            std::cout << "Input: " << text << std::endl;
            std::cout << "Output: " << processInput(text, op) << std::endl;
        }
    }
    
    // Test case 3: Character count
    std::cout << "\\nTest 3: Character count" << std::endl;
    {
        std::string text = "Secure Code";
        int op = 3;
        if (validateAlphanumeric(text) && validateStringLength(text, 1, 100)) {
            std::cout << "Input: " << text << std::endl;
            std::cout << "Output: " << processInput(text, op) << std::endl;
        }
    }
    
    // Test case 4: Invalid input - special characters rejected
    std::cout << "\\nTest 4: Invalid input with special characters" << std::endl;
    {
        std::string text = "Hello@World!";
        if (!validateAlphanumeric(text)) {
            std::cout << "Input rejected: Contains invalid characters" << std::endl;
        }
    }
    
    // Test case 5: Invalid input - exceeds length
    std::cout << "\\nTest 5: Invalid input exceeds maximum length" << std::endl;
    {
        std::string text(150, 'A'); // String longer than 100 chars
        if (!validateStringLength(text, 1, 100)) {
            std::cout << "Input rejected: Exceeds maximum length of 100 characters" << std::endl;
        }
    }
    
    return 0;
}
