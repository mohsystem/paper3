#include <iostream>
#include <string>
#include <cctype>

/**
 * @brief Validates a PIN to be exactly 4 or 6 digits.
 * @param pin The PIN string to validate.
 * @return true if the PIN is valid, false otherwise.
 */
bool validatePin(const std::string& pin) {
    if (pin.length() != 4 && pin.length() != 6) {
        return false;
    }
    for (char const &c : pin) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

int main() {
    // 5 test cases
    std::string testCases[] = {"1234", "12345", "a234", "123456", "123"};
    
    // std::boolalpha ensures that boolean values are printed as "true" or "false"
    std::cout << std::boolalpha;

    for (const std::string& pin : testCases) {
        std::cout << "\"" << pin << "\"   -->  " << validatePin(pin) << std::endl;
    }
    
    return 0;
}