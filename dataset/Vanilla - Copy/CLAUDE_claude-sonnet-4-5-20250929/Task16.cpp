
#include <iostream>
#include <string>

bool solution(std::string str, std::string ending) {
    if (ending.length() > str.length()) {
        return false;
    }
    return str.substr(str.length() - ending.length()) == ending;
}

int main() {
    // Test case 1
    std::cout << std::boolalpha << solution("abc", "bc") << std::endl; // true
    
    // Test case 2
    std::cout << std::boolalpha << solution("abc", "d") << std::endl; // false
    
    // Test case 3
    std::cout << std::boolalpha << solution("hello world", "world") << std::endl; // true
    
    // Test case 4
    std::cout << std::boolalpha << solution("test", "") << std::endl; // true
    
    // Test case 5
    std::cout << std::boolalpha << solution("programming", "ing") << std::endl; // true
    
    return 0;
}
