
#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> number(const std::vector<std::string>& lines) {
    std::vector<std::string> result;
    
    for (size_t i = 0; i < lines.size(); i++) {
        result.push_back(std::to_string(i + 1) + ": " + lines[i]);
    }
    
    return result;
}

int main() {
    // Test case 1: Empty list
    std::cout << "Test 1: ";
    auto test1 = number({});
    for (const auto& line : test1) {
        std::cout << line << " ";
    }
    std::cout << std::endl;
    
    // Test case 2: Simple list
    std::cout << "Test 2: ";
    auto test2 = number({"a", "b", "c"});
    for (const auto& line : test2) {
        std::cout << line << " ";
    }
    std::cout << std::endl;
    
    // Test case 3: Single element
    std::cout << "Test 3: ";
    auto test3 = number({"Hello World"});
    for (const auto& line : test3) {
        std::cout << line << " ";
    }
    std::cout << std::endl;
    
    // Test case 4: Multiple lines with different content
    std::cout << "Test 4: ";
    auto test4 = number({"line one", "line two", "line three", "line four"});
    for (const auto& line : test4) {
        std::cout << line << " ";
    }
    std::cout << std::endl;
    
    // Test case 5: Lines with special characters
    std::cout << "Test 5: ";
    auto test5 = number({"Hello!", "How are you?", "I'm fine, thanks."});
    for (const auto& line : test5) {
        std::cout << line << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
