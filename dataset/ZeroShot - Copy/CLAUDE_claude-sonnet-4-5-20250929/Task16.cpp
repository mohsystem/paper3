
#include <iostream>
#include <string>

bool solution(const std::string& str, const std::string& ending) {
    if (str.length() < ending.length()) {
        return false;
    }
    return str.compare(str.length() - ending.length(), ending.length(), ending) == 0;
}

int main() {
    // Test cases
    std::cout << std::boolalpha;
    std::cout << solution("abc", "bc") << std::endl;      // true
    std::cout << solution("abc", "d") << std::endl;       // false
    std::cout << solution("hello", "lo") << std::endl;    // true
    std::cout << solution("test", "") << std::endl;       // true
    std::cout << solution("", "test") << std::endl;       // false
    
    return 0;
}
