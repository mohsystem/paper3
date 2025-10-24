
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
        } else {
            if (capitalizeNext && std::isalpha(c)) {
                result += std::toupper(c);
                capitalizeNext = false;
            } else {
                result += c;
            }
        }
    }
    
    return result;
}

int main() {
    // Test cases
    std::cout << toJadenCase("How can mirrors be real if our eyes aren't real") << std::endl;
    std::cout << toJadenCase("the quick brown fox jumps over the lazy dog") << std::endl;
    std::cout << toJadenCase("i love programming") << std::endl;
    std::cout << toJadenCase("") << std::endl;
    std::cout << toJadenCase("test case five") << std::endl;
    
    return 0;
}
