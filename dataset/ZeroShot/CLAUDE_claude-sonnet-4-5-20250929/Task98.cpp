
#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <stdexcept>

class Task98 {
public:
    static bool convertToInteger(const std::string& input, int& result) {
        try {
            std::string trimmed = input;
            trimmed.erase(0, trimmed.find_first_not_of(" \\t\\n\\r"));
            trimmed.erase(trimmed.find_last_not_of(" \\t\\n\\r") + 1);
            
            size_t pos = 0;
            result = std::stoi(trimmed, &pos);
            
            if (pos != trimmed.length()) {
                std::cout << "Error: Invalid input. Cannot convert '" << input << "' to integer." << std::endl;
                return false;
            }
            return true;
        } catch (const std::invalid_argument& e) {
            std::cout << "Error: Invalid input. Cannot convert '" << input << "' to integer." << std::endl;
            return false;
        } catch (const std::out_of_range& e) {
            std::cout << "Error: Number out of range. Cannot convert '" << input << "' to integer." << std::endl;
            return false;
        }
    }
};

int main() {
    // Test cases
    std::string testCases[] = {"123", "-456", "0", "abc", "12.34"};
    
    std::cout << "Running test cases:" << std::endl;
    for (const auto& testCase : testCases) {
        std::cout << "Input: '" << testCase << "' -> ";
        int result;
        if (Task98::convertToInteger(testCase, result)) {
            std::cout << "Output: " << result << std::endl;
        }
    }
    
    // Interactive input
    std::cout << "\\nEnter a value to convert to integer (or 'exit' to quit):" << std::endl;
    std::string input;
    
    while (std::getline(std::cin, input)) {
        if (input == "exit") {
            break;
        }
        int result;
        if (Task98::convertToInteger(input, result)) {
            std::cout << "Successfully converted to: " << result << std::endl;
        }
        std::cout << "Enter another value (or 'exit' to quit):" << std::endl;
    }
    
    return 0;
}
