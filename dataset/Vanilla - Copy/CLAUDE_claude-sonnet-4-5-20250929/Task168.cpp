
#include <iostream>
#include <string>
#include <vector>

std::string tweakLetters(std::string str, std::vector<int> tweaks) {
    std::string result = "";
    for (int i = 0; i < str.length(); i++) {
        char tweakedChar = str[i] + tweaks[i];
        result += tweakedChar;
    }
    return result;
}

int main() {
    // Test case 1
    std::cout << tweakLetters("apple", {0, 1, -1, 0, -1}) << std::endl;  // "aqold"
    
    // Test case 2
    std::cout << tweakLetters("many", {0, 0, 0, -1}) << std::endl;  // "manx"
    
    // Test case 3
    std::cout << tweakLetters("rhino", {1, 1, 1, 1, 1}) << std::endl;  // "sijop"
    
    // Test case 4
    std::cout << tweakLetters("hello", {0, 0, 0, 0, 0}) << std::endl;  // "hello"
    
    // Test case 5
    std::cout << tweakLetters("code", {-1, 1, -1, 1}) << std::endl;  // "bndf"
    
    return 0;
}
