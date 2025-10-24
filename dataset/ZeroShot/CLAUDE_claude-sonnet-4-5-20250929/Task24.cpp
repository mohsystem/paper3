
#include <iostream>
#include <string>

bool validatePin(const std::string& pin) {
    int length = pin.length();
    if (length != 4 && length != 6) {
        return false;
    }
    
    for (int i = 0; i < length; i++) {
        if (pin[i] < '0' || pin[i] > '9') {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test cases
    std::cout << std::boolalpha;
    std::cout << validatePin("1234") << std::endl;   // true
    std::cout << validatePin("123456") << std::endl; // true
    std::cout << validatePin("12345") << std::endl;  // false
    std::cout << validatePin("a234") << std::endl;   // false
    std::cout << validatePin("") << std::endl;       // false
    
    return 0;
}
