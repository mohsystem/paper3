
#include <iostream>
#include <string>
#include <limits>
#include <cctype>
#include <sstream>

class Task98 {
public:
    static bool convertToInteger(const std::string& input, int& result) {
        if (input.empty()) {
            return false;
        }
        
        // Trim whitespace
        std::string trimmed = input;
        size_t start = trimmed.find_first_not_of(" \\t\\n\\r");
        size_t end = trimmed.find_last_not_of(" \\t\\n\\r");
        
        if (start == std::string::npos) {
            return false;
        }
        
        trimmed = trimmed.substr(start, end - start + 1);
        
        // Use stringstream for safe conversion
        std::stringstream ss(trimmed);
        long long temp;
        
        if (!(ss >> temp) || !ss.eof()) {
            return false;
        }
        
        // Check for overflow
        if (temp > std::numeric_limits<int>::max() || 
            temp < std::numeric_limits<int>::min()) {
            return false;
        }
        
        result = static_cast<int>(temp);
        return true;
    }
    
    static void runTests() {
        std::string testCases[] = {"123", "-456", "0", "abc", "2147483648"};
        int numTests = 5;
        
        std::cout << "Running test cases:" << std::endl;
        for (int i = 0; i < numTests; i++) {
            int result;
            if (convertToInteger(testCases[i], result)) {
                std::cout << "Input: '" << testCases[i] << "' -> Output: " 
                         << result << std::endl;
            } else {
                std::cout << "Input: '" << testCases[i] << "' -> Invalid input" 
                         << std::endl;
            }
        }
    }
};

int main() {
    Task98::runTests();
    
    std::cout << "\\n--- Interactive Mode ---" << std::endl;
    std::cout << "Enter a value to convert to integer: ";
    
    std::string userInput;
    if (std::getline(std::cin, userInput)) {
        int result;
        if (Task98::convertToInteger(userInput, result)) {
            std::cout << "Successfully converted to integer: " << result << std::endl;
        } else {
            std::cout << "Error: Invalid input. Please enter a valid integer." << std::endl;
        }
    }
    
    return 0;
}
