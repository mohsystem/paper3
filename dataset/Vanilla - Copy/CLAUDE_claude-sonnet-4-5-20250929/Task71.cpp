
#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <vector>

class Task71 {
public:
    static bool convertToInteger(const std::string& input, int& result) {
        std::string trimmed = input;
        trimmed.erase(0, trimmed.find_first_not_of(" \\t\\n\\r"));
        trimmed.erase(trimmed.find_last_not_of(" \\t\\n\\r") + 1);
        
        std::stringstream ss(trimmed);
        ss >> result;
        
        if (ss.fail() || !ss.eof()) {
            return false;
        }
        return true;
    }
    
    static void runTests() {
        std::vector<std::string> testCases = {"123", "-456", "0", "abc", "12.34"};
        
        std::cout << "Running test cases:" << std::endl;
        for (const auto& testCase : testCases) {
            int result;
            if (convertToInteger(testCase, result)) {
                std::cout << "Input: '" << testCase << "' -> Output: " << result << std::endl;
            } else {
                std::cout << "Input: '" << testCase << "' -> Output: Invalid (cannot convert)" << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int result;
        if (Task71::convertToInteger(argv[1], result)) {
            std::cout << "Converted integer: " << result << std::endl;
        } else {
            std::cout << "Invalid integer input: " << argv[1] << std::endl;
        }
    } else {
        Task71::runTests();
    }
    
    return 0;
}
