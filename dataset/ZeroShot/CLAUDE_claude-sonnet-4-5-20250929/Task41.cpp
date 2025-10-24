
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

const size_t MAX_INPUT_LENGTH = 1024;

class Task41 {
public:
    static std::string processString(const std::string& input) {
        // Validate input length to prevent buffer overflow
        if (input.length() > MAX_INPUT_LENGTH) {
            return "Error: Input exceeds maximum length of " + std::to_string(MAX_INPUT_LENGTH);
        }
        
        // Create a safe copy of the input
        std::string processed = input;
        
        // Trim leading whitespace
        processed.erase(processed.begin(), 
                       std::find_if(processed.begin(), processed.end(), 
                       [](unsigned char ch) { return !std::isspace(ch); }));
        
        // Trim trailing whitespace
        processed.erase(std::find_if(processed.rbegin(), processed.rend(), 
                       [](unsigned char ch) { return !std::isspace(ch); }).base(), 
                       processed.end());
        
        // Convert to uppercase
        std::transform(processed.begin(), processed.end(), processed.begin(),
                      [](unsigned char c) { return std::toupper(c); });
        
        return "Processed: " + processed;
    }
};

int main() {
    // Test case 1: Normal string
    std::cout << "Test 1: " << Task41::processString("Hello World") << std::endl;
    
    // Test case 2: Empty string
    std::cout << "Test 2: " << Task41::processString("") << std::endl;
    
    // Test case 3: String with whitespace
    std::cout << "Test 3: " << Task41::processString("  C++ Programming  ") << std::endl;
    
    // Test case 4: Single character
    std::cout << "Test 4: " << Task41::processString("a") << std::endl;
    
    // Test case 5: Very long string
    std::string longString(2000, 'A');
    std::cout << "Test 5: " << Task41::processString(longString) << std::endl;
    
    return 0;
}
