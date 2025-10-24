
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

class Task113 {
public:
    // Method to sanitize input string
    static std::string sanitizeInput(const std::string& input) {
        std::string sanitized;
        sanitized.reserve(input.length());
        
        for (char c : input) {
            // Remove control characters while preserving valid input
            if ((c >= 32 && c <= 126) || c == '\\t' || c == '\\n' || c == '\\r') {
                sanitized += c;
            }
        }
        
        return sanitized;
    }
    
    // Method to safely concatenate strings with input validation
    static std::string concatenateStrings(const std::vector<std::string>& strings) {
        if (strings.empty()) {
            return "";
        }
        
        std::ostringstream result;
        
        for (const auto& str : strings) {
            // Sanitize input to prevent injection attacks
            std::string sanitized = sanitizeInput(str);
            result << sanitized;
        }
        
        return result.str();
    }
};

int main() {
    std::cout << "=== Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Normal strings
    std::vector<std::string> test1 = {"Hello", " ", "World", "!"};
    std::cout << "Test 1 - Normal strings: " << Task113::concatenateStrings(test1) << std::endl;
    
    // Test Case 2: Empty list
    std::vector<std::string> test2;
    std::cout << "Test 2 - Empty list: '" << Task113::concatenateStrings(test2) << "'" << std::endl;
    
    // Test Case 3: Strings with numbers and special characters
    std::vector<std::string> test3 = {"User", "123", "@", "Example"};
    std::cout << "Test 3 - Mixed content: " << Task113::concatenateStrings(test3) << std::endl;
    
    // Test Case 4: Strings with empty values
    std::vector<std::string> test4 = {"Start", "", "End"};
    std::cout << "Test 4 - With empty values: " << Task113::concatenateStrings(test4) << std::endl;
    
    // Test Case 5: Single string
    std::vector<std::string> test5 = {"SingleString"};
    std::cout << "Test 5 - Single string: " << Task113::concatenateStrings(test5) << std::endl;
    
    // Interactive input option
    std::cout << "\\n=== Interactive Mode ===" << std::endl;
    int count;
    std::cout << "Enter number of strings to concatenate: ";
    
    if (!(std::cin >> count) || count < 0 || count > 1000) {
        std::cout << "Invalid count. Please enter a value between 0 and 1000." << std::endl;
        return 1;
    }
    
    std::cin.ignore(); // Clear newline from buffer
    
    std::vector<std::string> userInputs;
    userInputs.reserve(count);
    
    for (int i = 0; i < count; i++) {
        std::string input;
        std::cout << "Enter string " << (i + 1) << ": ";
        std::getline(std::cin, input);
        userInputs.push_back(input);
    }
    
    std::string result = Task113::concatenateStrings(userInputs);
    std::cout << "\\nConcatenated Result: " << result << std::endl;
    
    return 0;
}
