
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>

// Function to add line numbers to a list of strings
// Security: Validates input size to prevent integer overflow and resource exhaustion
// Returns a new vector with numbered lines in format "n: string"
std::vector<std::string> number(const std::vector<std::string>& lines) {
    // Security: Check for excessively large input that could cause integer overflow
    // or resource exhaustion. Limit to reasonable text editor line count.
    if (lines.size() > 10000000) {
        throw std::invalid_argument("Input exceeds maximum allowed line count");
    }
    
    std::vector<std::string> result;
    // Security: Reserve capacity to avoid multiple reallocations
    result.reserve(lines.size());
    
    // Security: Use size_t for loop counter to match vector::size() type
    // and prevent signed/unsigned comparison issues
    for (size_t i = 0; i < lines.size(); ++i) {
        // Security: Check for integer overflow when converting size_t to line number
        // size_t max is much larger than reasonable line numbers
        if (i >= static_cast<size_t>(std::numeric_limits<int>::max())) {
            throw std::overflow_error("Line number exceeds maximum integer value");
        }
        
        // Security: Validate individual line length to prevent memory exhaustion
        if (lines[i].length() > 1000000) {
            throw std::invalid_argument("Individual line exceeds maximum allowed length");
        }
        
        // Security: Use std::to_string for safe integer-to-string conversion
        // No format string vulnerabilities possible
        std::string numbered_line = std::to_string(i + 1) + ": " + lines[i];
        result.push_back(numbered_line);
    }
    
    return result;
}

// Test driver with 5 test cases
int main() {
    try {
        // Test case 1: Empty list
        std::vector<std::string> test1 = {};
        std::vector<std::string> result1 = number(test1);
        std::cout << "Test 1 (empty): ";
        if (result1.empty()) {
            std::cout << "PASS" << std::endl;
        } else {
            std::cout << "FAIL" << std::endl;
        }
        
        // Test case 2: Basic example
        std::vector<std::string> test2 = {"a", "b", "c"};
        std::vector<std::string> result2 = number(test2);
        std::cout << "Test 2 (a,b,c): ";
        for (const auto& line : result2) {
            std::cout << line << " ";
        }
        std::cout << std::endl;
        
        // Test case 3: Single line
        std::vector<std::string> test3 = {"hello world"};
        std::vector<std::string> result3 = number(test3);
        std::cout << "Test 3 (single): ";
        for (const auto& line : result3) {
            std::cout << line << std::endl;
        }
        
        // Test case 4: Lines with special characters
        std::vector<std::string> test4 = {"line!@#", "line$%^", "line&*()"};
        std::vector<std::string> result4 = number(test4);
        std::cout << "Test 4 (special chars): ";
        for (const auto& line : result4) {
            std::cout << line << " ";
        }
        std::cout << std::endl;
        
        // Test case 5: Multiple lines
        std::vector<std::string> test5 = {"first", "second", "third", "fourth", "fifth"};
        std::vector<std::string> result5 = number(test5);
        std::cout << "Test 5 (five lines): ";
        for (const auto& line : result5) {
            std::cout << line << " ";
        }
        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
