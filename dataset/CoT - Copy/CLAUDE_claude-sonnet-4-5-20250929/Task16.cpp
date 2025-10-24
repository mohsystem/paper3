
#include <iostream>
#include <string>

bool solution(const std::string& str, const std::string& ending) {
    // Check if ending is longer than the string
    if (ending.length() > str.length()) {
        return false;
    }
    
    // Compare the end of str with ending using safe comparison
    // This uses std::string's built-in comparison which is secure
    return str.compare(str.length() - ending.length(), ending.length(), ending) == 0;
}

int main() {
    // Test case 1: Basic positive case
    std::cout << std::boolalpha << solution("abc", "bc") << std::endl; // true
    
    // Test case 2: Basic negative case
    std::cout << std::boolalpha << solution("abc", "d") << std::endl; // false
    
    // Test case 3: Empty ending (edge case)
    std::cout << std::boolalpha << solution("abc", "") << std::endl; // true
    
    // Test case 4: Ending longer than string
    std::cout << std::boolalpha << solution("ab", "abc") << std::endl; // false
    
    // Test case 5: Same strings
    std::cout << std::boolalpha << solution("abc", "abc") << std::endl; // true
    
    return 0;
}
