
#include <iostream>
#include <string>
#include <limits>
#include <cerrno>
#include <cstdlib>

// Function to safely convert string to integer
// Returns true if conversion succeeds, false otherwise
bool convertStringToInt(const std::string& input, int& result) {
    // Rule#3: Validate input - check for empty string
    if (input.empty()) {
        std::cerr << "Error: Empty input string\\n";
        return false;
    }
    
    // Rule#3: Validate input - check string length to prevent overflow
    // Maximum int is typically 10-11 digits including sign
    if (input.length() > 15) {
        std::cerr << "Error: Input string too long\\n";
        return false;
    }
    
    // Rule#3: Validate that string contains only valid characters
    size_t startPos = 0;
    if (input[0] == '-' || input[0] == '+') {
        startPos = 1;
        if (input.length() == 1) {
            std::cerr << "Error: Sign character without digits\\n";
            return false;
        }
    }
    
    for (size_t i = startPos; i < input.length(); ++i) {
        if (input[i] < '0' || input[i] > '9') {
            std::cerr << "Error: Invalid character in input\\n";
            return false;
        }
    }
    
    // Rule#2: Use safer conversion with error checking
    errno = 0;
    char* endPtr = nullptr;
    // Make a copy to avoid const_cast issues
    long value = std::strtol(input.c_str(), &endPtr, 10);
    
    // Rule#6: Check for conversion errors
    if (errno == ERANGE || value < std::numeric_limits<int>::min() || 
        value > std::numeric_limits<int>::max()) {
        std::cerr << "Error: Integer overflow/underflow\\n";
        return false;
    }
    
    // Check if entire string was consumed
    if (endPtr != input.c_str() + input.length()) {
        std::cerr << "Error: Partial conversion only\\n";
        return false;
    }
    
    result = static_cast<int>(value);
    return true;
}

int main() {
    // Test cases
    std::string testCases[] = {
        "12345",
        "-9876",
        "0",
        "2147483647",
        "invalid123"
    };
    
    std::cout << "Running test cases:\\n";
    for (int i = 0; i < 5; ++i) {
        std::cout << "\\nTest " << (i + 1) << ": Input = \\"" << testCases[i] << "\\"\\n";
        int result = 0;
        if (convertStringToInt(testCases[i], result)) {
            std::cout << "Success: Converted to " << result << "\\n";
        } else {
            std::cout << "Failed to convert\\n";
        }
    }
    
    // Rule#3: Interactive user input with validation
    std::cout << "\\n--- Interactive Mode ---\\n";
    std::cout << "Enter an integer (or 'quit' to exit): ";
    
    std::string userInput;
    // Rule#4: Use safe input method with size limits
    if (std::getline(std::cin, userInput)) {
        // Rule#3: Validate input length before processing
        if (userInput.length() > 1000) {
            std::cerr << "Error: Input too long (max 1000 characters)\\n";
            return 1;
        }
        
        if (userInput == "quit") {
            return 0;
        }
        
        int result = 0;
        if (convertStringToInt(userInput, result)) {
            std::cout << "Successfully converted to integer: " << result << "\\n";
        } else {
            std::cerr << "Conversion failed\\n";
            return 1;
        }
    }
    
    return 0;
}
