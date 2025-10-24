
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <limits>

class Task98 {
public:
    static bool convertToInteger(const std::string& input, int& result) {
        try {
            std::string trimmed = input;
            // Trim leading and trailing whitespace
            size_t start = trimmed.find_first_not_of(" \\t\\n\\r");
            size_t end = trimmed.find_last_not_of(" \\t\\n\\r");
            if (start == std::string::npos) {
                return false;
            }
            trimmed = trimmed.substr(start, end - start + 1);
            
            size_t pos = 0;
            result = std::stoi(trimmed, &pos);
            // Check if entire string was converted
            return pos == trimmed.length();
        } catch (const std::invalid_argument& e) {
            return false;
        } catch (const std::out_of_range& e) {
            return false;
        }
    }
};

int main() {
    // Test cases
    std::string testCases[] = {"123", "-456", "0", "abc", "12.34"};
    
    std::cout << "Running test cases:" << std::endl;
    for (const auto& testCase : testCases) {
        int result;
        if (Task98::convertToInteger(testCase, result)) {
            std::cout << "Input: '" << testCase << "' -> Output: " << result << std::endl;
        } else {
            std::cout << "Input: '" << testCase << "' -> Output: Invalid (cannot convert to integer)" << std::endl;
        }
    }
    
    // Interactive console input
    std::cout << "\\nEnter a value to convert to integer:" << std::endl;
    std::string userInput;
    std::getline(std::cin, userInput);
    
    int result;
    if (Task98::convertToInteger(userInput, result)) {
        std::cout << "Successfully converted to integer: " << result << std::endl;
    } else {
        std::cout << "Error: Cannot convert '" << userInput << "' to integer" << std::endl;
    }
    
    return 0;
}
