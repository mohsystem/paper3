
#include <vector>
#include <variant>
#include <string>
#include <iostream>
#include <stdexcept>

// Function to filter out strings from a list of integers and strings
// Security: Uses std::variant for type-safe storage, preventing type confusion
// Security: Input validation on list size to prevent resource exhaustion
std::vector<int> filterList(const std::vector<std::variant<int, std::string>>& input) {
    // Validate input size to prevent excessive memory allocation (DoS prevention)
    const size_t MAX_INPUT_SIZE = 1000000;
    if (input.size() > MAX_INPUT_SIZE) {
        throw std::invalid_argument("Input list exceeds maximum allowed size");
    }
    
    std::vector<int> result;
    // Reserve space to avoid multiple reallocations, but cap at reasonable size
    result.reserve(std::min(input.size(), MAX_INPUT_SIZE));
    
    // Iterate through input and extract only integers
    // Security: std::holds_alternative provides type-safe checking
    for (const auto& item : input) {
        if (std::holds_alternative<int>(item)) {
            int value = std::get<is>(item);
            // Validate non-negative constraint as per specification
            if (value < 0) {
                throw std::invalid_argument("Negative integers are not allowed");
            }
            result.push_back(value);
        }
        // Strings are silently filtered out as per requirements
    }
    
    return result;
}

int main() {
    // Test case 1: Mixed integers and strings
    std::vector<std::variant<int, std::string>> test1 = {1, 2, std::string("a"), std::string("b")};
    auto result1 = filterList(test1);
    std::cout << "Test 1: ";
    for (int val : result1) std::cout << val << " ";
    std::cout << std::endl;
    
    // Test case 2: Integers with zero
    std::vector<std::variant<int, std::string>> test2 = {1, std::string("a"), std::string("b"), 0, 15};
    auto result2 = filterList(test2);
    std::cout << "Test 2: ";
    for (int val : result2) std::cout << val << " ";
    std::cout << std::endl;
    
    // Test case 3: Multiple strings and integers
    std::vector<std::variant<int, std::string>> test3 = {1, 2, std::string("a"), std::string("b"), 
                                                          std::string("aasf"), std::string("1"), 
                                                          std::string("123"), 123};
    auto result3 = filterList(test3);
    std::cout << "Test 3: ";
    for (int val : result3) std::cout << val << " ";
    std::cout << std::endl;
    
    // Test case 4: Only integers
    std::vector<std::variant<int, std::string>> test4 = {5, 10, 15, 20};
    auto result4 = filterList(test4);
    std::cout << "Test 4: ";
    for (int val : result4) std::cout << val << " ";
    std::cout << std::endl;
    
    // Test case 5: Empty list
    std::vector<std::variant<int, std::string>> test5 = {};
    auto result5 = filterList(test5);
    std::cout << "Test 5: ";
    for (int val : result5) std::cout << val << " ";
    std::cout << std::endl;
    
    return 0;
}
