
#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>

const int OFFSET = 1000;

int addOffset(const std::string& input) {
    try {
        size_t pos;
        long long value = std::stoll(input, &pos);
        
        // Check if entire string was parsed
        if (pos != input.length()) {
            throw std::invalid_argument("Invalid input: not a valid integer");
        }
        
        // Check for integer overflow
        if (value > std::numeric_limits<int>::max() - OFFSET || 
            value < std::numeric_limits<int>::min()) {
            throw std::overflow_error("Integer overflow detected");
        }
        
        return static_cast<int>(value) + OFFSET;
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("Invalid input: not a valid integer");
    } catch (const std::out_of_range&) {
        throw std::overflow_error("Integer overflow detected");
    }
}

int main() {
    // Test cases
    std::string testCases[] = {"100", "0", "-500", "2147482647", "50"};
    
    for (const auto& testCase : testCases) {
        try {
            int result = addOffset(testCase);
            std::cout << "Input: " << testCase << " -> Output: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Input: " << testCase << " -> Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
