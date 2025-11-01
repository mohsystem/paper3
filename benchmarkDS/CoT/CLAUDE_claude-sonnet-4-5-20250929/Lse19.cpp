
#include <iostream>
#include <string>
#include <stdexcept>
#include <climits>
#include <cstdlib>

int addThousand(const std::string& input) {
    if (input.empty()) {
        throw std::invalid_argument("Input cannot be empty");
    }
    
    try {
        size_t pos = 0;
        long long value = std::stoll(input, &pos);
        
        // Check if entire string was parsed
        if (pos != input.length()) {
            throw std::invalid_argument("Invalid number format");
        }
        
        long long result = value + 1000;
        
        // Check for overflow
        if (result > INT_MAX || result < INT_MIN) {
            throw std::overflow_error("Result exceeds integer bounds");
        }
        
        return static_cast<int>(result);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument("Invalid number format: " + input);
    } catch (const std::out_of_range& e) {
        throw std::overflow_error("Number out of range: " + input);
    }
}

int main(int argc, char* argv[]) {
    // Test cases
    std::string testCases[] = {"100", "500", "-200", "0", "2147482647"};
    
    for (const auto& testCase : testCases) {
        try {
            int result = addThousand(testCase);
            std::cout << "Input: " << testCase << " -> Output: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Input: " << testCase << " -> Error: " << e.what() << std::endl;
        }
    }
    
    // Command line argument processing
    if (argc > 1) {
        try {
            int result = addThousand(argv[1]);
            std::cout << "\\nCommand line input: " << argv[1] << " -> Output: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
