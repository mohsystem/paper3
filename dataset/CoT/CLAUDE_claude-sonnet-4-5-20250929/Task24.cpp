
#include <iostream>
#include <string>
#include <cctype>

bool validatePin(const std::string& pin) {
    // Empty check
    if (pin.empty()) {
        return false;
    }
    
    // Length validation - must be exactly 4 or 6
    size_t length = pin.length();
    if (length != 4 && length != 6) {
        return false;
    }
    
    // Validate each character is a digit
    for (size_t i = 0; i < length; i++) {
        if (!std::isdigit(static_cast<unsigned char>(pin[i]))) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test cases
    std::cout << "Test 1 - '1234': " << (validatePin("1234") ? "true" : "false") << std::endl;      // true
    std::cout << "Test 2 - '12345': " << (validatePin("12345") ? "true" : "false") << std::endl;    // false
    std::cout << "Test 3 - 'a234': " << (validatePin("a234") ? "true" : "false") << std::endl;      // false
    std::cout << "Test 4 - '123456': " << (validatePin("123456") ? "true" : "false") << std::endl;  // true
    std::cout << "Test 5 - '': " << (validatePin("") ? "true" : "false") << std::endl;              // false
    
    return 0;
}
