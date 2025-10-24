
#include <iostream>
#include <string>
#include <vector>

std::string concatenateStrings(const std::vector<std::string>& strings) {
    std::string result = "";
    for (const auto& str : strings) {
        result += str;
    }
    return result;
}

int main() {
    // Test case 1
    std::vector<std::string> test1 = {"Hello", " ", "World"};
    std::cout << "Test 1: " << concatenateStrings(test1) << std::endl;
    
    // Test case 2
    std::vector<std::string> test2 = {"Java", "Programming", "Language"};
    std::cout << "Test 2: " << concatenateStrings(test2) << std::endl;
    
    // Test case 3
    std::vector<std::string> test3 = {"One", "Two", "Three", "Four"};
    std::cout << "Test 3: " << concatenateStrings(test3) << std::endl;
    
    // Test case 4
    std::vector<std::string> test4 = {"", "Empty", "", "Strings"};
    std::cout << "Test 4: " << concatenateStrings(test4) << std::endl;
    
    // Test case 5
    std::vector<std::string> test5 = {"Single"};
    std::cout << "Test 5: " << concatenateStrings(test5) << std::endl;
    
    return 0;
}
