
#include <iostream>
#include <string>
#include <cctype>

std::string toJadenCase(const std::string& phrase) {
    if (phrase.empty()) {
        return "";
    }
    
    std::string result;
    result.reserve(phrase.length());
    bool capitalizeNext = true;
    
    for (size_t i = 0; i < phrase.length(); i++) {
        char c = phrase[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            result += c;
            capitalizeNext = true;
        } else if (capitalizeNext) {
            result += std::toupper(static_cast<unsigned char>(c));
            capitalizeNext = false;
        } else {
            result += c;
        }
    }
    
    return result;
}

int main() {
    std::cout << "Test 1: " << toJadenCase("How can mirrors be real if our eyes aren't real") << std::endl;
    std::cout << "Test 2: " << toJadenCase("this is a test") << std::endl;
    std::cout << "Test 3: " << toJadenCase("i love programming") << std::endl;
    std::cout << "Test 4: " << toJadenCase("") << std::endl;
    std::cout << "Test 5: " << toJadenCase("hello world") << std::endl;
    return 0;
}
