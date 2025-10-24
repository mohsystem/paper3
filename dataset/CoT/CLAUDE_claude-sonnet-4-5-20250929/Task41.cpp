
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>

const int MAX_LENGTH = 1024;

class Task41 {
public:
    static std::string processString(const char* input) {
        if (input == nullptr) {
            return "Error: Null input";
        }
        
        size_t length = strnlen(input, MAX_LENGTH + 1);
        
        if (length > MAX_LENGTH) {
            return "Error: Input exceeds maximum length of " + std::to_string(MAX_LENGTH);
        }
        
        // Safe string processing using std::string
        std::string safeInput(input, length);
        
        // Trim whitespace
        safeInput.erase(0, safeInput.find_first_not_of(" \\t\\n\\r"));
        safeInput.erase(safeInput.find_last_not_of(" \\t\\n\\r") + 1);
        
        std::string result = "Processed: " + safeInput + 
                           " (Length: " + std::to_string(length) + ")";
        
        return result;
    }
    
    static std::string processString(const std::string& input) {
        if (input.length() > MAX_LENGTH) {
            return "Error: Input exceeds maximum length of " + std::to_string(MAX_LENGTH);
        }
        
        std::string trimmed = input;
        trimmed.erase(0, trimmed.find_first_not_of(" \\t\\n\\r"));
        trimmed.erase(trimmed.find_last_not_of(" \\t\\n\\r") + 1);
        
        return "Processed: " + trimmed + " (Length: " + std::to_string(input.length()) + ")";
    }
};

int main() {
    // Test case 1: Normal input
    std::cout << "Test 1: " << Task41::processString("Hello World") << std::endl;
    
    // Test case 2: Empty string
    std::cout << "Test 2: " << Task41::processString("") << std::endl;
    
    // Test case 3: String with whitespace
    std::cout << "Test 3: " << Task41::processString("  Test String  ") << std::endl;
    
    // Test case 4: Null input
    std::cout << "Test 4: " << Task41::processString((const char*)nullptr) << std::endl;
    
    // Test case 5: Long string
    std::string longString(500, 'A');
    std::cout << "Test 5: " << Task41::processString(longString) << std::endl;
    
    // Interactive mode
    std::cout << "\\nEnter a string to process (or 'exit' to quit):" << std::endl;
    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "exit") {
            break;
        }
        std::cout << Task41::processString(input) << std::endl;
        std::cout << "Enter another string (or 'exit' to quit):" << std::endl;
    }
    
    return 0;
}
