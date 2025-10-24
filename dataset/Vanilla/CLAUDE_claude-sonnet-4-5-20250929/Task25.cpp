
#include <vector>
#include <string>
#include <iostream>

std::vector<std::string> number(const std::vector<std::string>& lines) {
    std::vector<std::string> result;
    for (size_t i = 0; i < lines.size(); i++) {
        result.push_back(std::to_string(i + 1) + ": " + lines[i]);
    }
    return result;
}

int main() {
    // Test case 1: Empty list
    std::vector<std::string> test1 = {};
    std::vector<std::string> result1 = number(test1);
    std::cout << "Test 1: [";
    for (size_t i = 0; i < result1.size(); i++) {
        std::cout << "\\"" << result1[i] << "\\"";
        if (i < result1.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    // Test case 2: Basic example
    std::vector<std::string> test2 = {"a", "b", "c"};
    std::vector<std::string> result2 = number(test2);
    std::cout << "Test 2: [";
    for (size_t i = 0; i < result2.size(); i++) {
        std::cout << "\\"" << result2[i] << "\\"";
        if (i < result2.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    // Test case 3: Single element
    std::vector<std::string> test3 = {"hello"};
    std::vector<std::string> result3 = number(test3);
    std::cout << "Test 3: [";
    for (size_t i = 0; i < result3.size(); i++) {
        std::cout << "\\"" << result3[i] << "\\"";
        if (i < result3.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    // Test case 4: Multiple words
    std::vector<std::string> test4 = {"hello world", "foo bar", "test"};
    std::vector<std::string> result4 = number(test4);
    std::cout << "Test 4: [";
    for (size_t i = 0; i < result4.size(); i++) {
        std::cout << "\\"" << result4[i] << "\\"";
        if (i < result4.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    // Test case 5: Empty strings
    std::vector<std::string> test5 = {"", "a", ""};
    std::vector<std::string> result5 = number(test5);
    std::cout << "Test 5: [";
    for (size_t i = 0; i < result5.size(); i++) {
        std::cout << "\\"" << result5[i] << "\\"";
        if (i < result5.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    return 0;
}
