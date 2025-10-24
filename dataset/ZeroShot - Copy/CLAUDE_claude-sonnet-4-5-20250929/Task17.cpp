
#include <iostream>
#include <string>
#include <cctype>

std::string toJadenCase(const std::string& phrase) {
    if (phrase.empty()) {
        return "";
    }
    
    std::string result;
    bool capitalizeNext = true;
    
    for (size_t i = 0; i < phrase.length(); i++) {
        char c = phrase[i];
        
        if (std::isspace(c)) {
            result += c;
            capitalizeNext = true;
        } else if (capitalizeNext) {
            result += std::toupper(c);
            capitalizeNext = false;
        } else {
            result += std::tolower(c);
        }
    }
    
    return result;
}

int main() {
    // Test cases
    std::cout << "Test 1: " << toJadenCase("How can mirrors be real if our eyes aren't real") << std::endl;
    std::cout << "Test 2: " << toJadenCase("the quick brown fox jumps over the lazy dog") << std::endl;
    std::cout << "Test 3: " << toJadenCase("I'm a single word") << std::endl;
    std::cout << "Test 4: " << toJadenCase("") << std::endl;
    std::cout << "Test 5: " << toJadenCase("hello world") << std::endl;
    
    return 0;
}
