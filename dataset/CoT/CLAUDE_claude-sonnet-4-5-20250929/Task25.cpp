
#include <iostream>
#include <vector>
#include <string>

class Task25 {
public:
    static std::vector<std::string> number(const std::vector<std::string>& lines) {
        std::vector<std::string> result;
        
        for (size_t i = 0; i < lines.size(); i++) {
            result.push_back(std::to_string(i + 1) + ": " + lines[i]);
        }
        
        return result;
    }
};

int main() {
    // Test case 1: Empty list
    std::vector<std::string> test1 = {};
    auto result1 = Task25::number(test1);
    std::cout << "Test 1: ";
    for (const auto& line : result1) std::cout << line << " ";
    std::cout << std::endl;
    
    // Test case 2: Basic example
    std::vector<std::string> test2 = {"a", "b", "c"};
    auto result2 = Task25::number(test2);
    std::cout << "Test 2: ";
    for (const auto& line : result2) std::cout << line << " ";
    std::cout << std::endl;
    
    // Test case 3: Single element
    std::vector<std::string> test3 = {"hello"};
    auto result3 = Task25::number(test3);
    std::cout << "Test 3: ";
    for (const auto& line : result3) std::cout << line << " ";
    std::cout << std::endl;
    
    // Test case 4: Strings with special characters
    std::vector<std::string> test4 = {"Hello, World!", "Line with spaces", "Special @#$%"};
    auto result4 = Task25::number(test4);
    std::cout << "Test 4: ";
    for (const auto& line : result4) std::cout << line << " ";
    std::cout << std::endl;
    
    // Test case 5: Empty strings
    std::vector<std::string> test5 = {"", "non-empty", ""};
    auto result5 = Task25::number(test5);
    std::cout << "Test 5: ";
    for (const auto& line : result5) std::cout << line << " ";
    std::cout << std::endl;
    
    return 0;
}
