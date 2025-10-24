
#include <iostream>
#include <string>
#include <cctype>

bool validatePin(const std::string& pin) {
    if (pin.length() != 4 && pin.length() != 6) {
        return false;
    }
    
    for (size_t i = 0; i < pin.length(); i++) {
        if (!std::isdigit(pin[i])) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test cases
    std::cout << std::boolalpha;
    std::cout << validatePin("1234") << std::endl;   // true
    std::cout << validatePin("12345") << std::endl;  // false
    std::cout << validatePin("a234") << std::endl;   // false
    std::cout << validatePin("123456") << std::endl; // true
    std::cout << validatePin("") << std::endl;       // false
    
    return 0;
}
