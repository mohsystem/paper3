#include <iostream>
#include <string>
#include <vector>

bool solution(const std::string& str, const std::string& ending) {
    if (ending.length() > str.length()) {
        return false;
    }
    return str.substr(str.length() - ending.length()) == ending;
}

int main() {
    // Test cases
    std::cout << std::boolalpha; // Print bools as "true" or "false"

    std::cout << "Test Case 1: solution('abc', 'bc')" << std::endl;
    std::cout << "Result: " << solution("abc", "bc") << std::endl; // Expected: true

    std::cout << "\nTest Case 2: solution('abc', 'd')" << std::endl;
    std::cout << "Result: " << solution("abc", "d") << std::endl; // Expected: false

    std::cout << "\nTest Case 3: solution('samurai', 'ai')" << std::endl;
    std::cout << "Result: " << solution("samurai", "ai") << std::endl; // Expected: true
    
    std::cout << "\nTest Case 4: solution('abc', '')" << std::endl;
    std::cout << "Result: " << solution("abc", "") << std::endl; // Expected: true

    std::cout << "\nTest Case 5: solution('test', 'testing')" << std::endl;
    std::cout << "Result: " << solution("test", "testing") << std::endl; // Expected: false

    return 0;
}