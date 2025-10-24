
#include <iostream>
#include <string>
#include <cstring>
#include <limits>

// Process string safely using C++ string class (automatic memory management)
// Returns processed string with length information
std::string processString(const std::string& input) {
    // Validate input is not empty
    if (input.empty()) {
        return "Error: Empty input";
    }
    
    // Validate input length to prevent excessive memory usage
    // Maximum 1MB input string to prevent DoS
    const size_t MAX_INPUT_LENGTH = 1024 * 1024;
    if (input.length() > MAX_INPUT_LENGTH) {
        return "Error: Input exceeds maximum length";
    }
    
    // Process the string safely - std::string handles memory automatically
    // Example processing: convert to uppercase and return with metadata
    std::string processed;
    processed.reserve(input.length()); // Pre-allocate to avoid multiple allocations
    
    for (char c : input) {
        // Validate each character is printable or whitespace
        if ((c >= 32 && c <= 126) || c == '\\n' || c == '\\t') {
            processed += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        } else {
            // Skip non-printable characters for safety
            processed += '?';
        }
    }
    
    return "Processed (" + std::to_string(processed.length()) + " chars): " + processed;
}

int main() {
    // Test case 1: Normal string
    std::string test1 = "Hello World";
    std::cout << "Test 1: " << processString(test1) << std::endl;
    
    // Test case 2: Empty string
    std::string test2 = "";
    std::cout << "Test 2: " << processString(test2) << std::endl;
    
    // Test case 3: String with special characters
    std::string test3 = "Test@123!";
    std::cout << "Test 3: " << processString(test3) << std::endl;
    
    // Test case 4: Long string (within limits)
    std::string test4(500, 'a');
    std::string result4 = processString(test4);
    std::cout << "Test 4: " << result4.substr(0, 50) << "..." << std::endl;
    
    // Test case 5: String with whitespace and tabs
    std::string test5 = "Hello\\tWorld\\nTest";
    std::cout << "Test 5: " << processString(test5) << std::endl;
    
    // Interactive mode - safe input reading
    std::cout << "\\nEnter a string to process (or 'quit' to exit): ";
    std::string userInput;
    
    // Use getline with stream state checking for safe input
    // Clear any error flags first
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
    
    while (std::getline(std::cin, userInput)) {
        if (userInput == "quit") {
            break;
        }
        
        // Process with automatic bounds checking via std::string
        std::cout << processString(userInput) << std::endl;
        std::cout << "\\nEnter another string (or 'quit' to exit): ";
    }
    
    return 0;
}
