#include <iostream>
#include <string>
#include <cctype>
#include <vector>

/**
 * Validates a PIN according to the rules:
 * - Must be exactly 4 or 6 digits long.
 * - Must contain only numeric digits.
 *
 * @param pin The PIN string to validate.
 * @return true if the PIN is valid, false otherwise.
 */
bool validatePin(const std::string& pin) {
    // Check if the length is 4 or 6
    if (pin.length() != 4 && pin.length() != 6) {
        return false;
    }

    // Check if all characters are digits
    for (char const &c : pin) {
        if (!std::isdigit(c)) {
            return false;
        }
    }

    return true;
}

int main() {
    // Test cases
    std::vector<std::string> testPins = {
        "1234",    // true
        "12345",   // false
        "a234",    // false
        "123456",  // true
        "12.0"     // false
    };

    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& pin : testPins) {
        std::cout << "\"" << pin << "\" --> " << std::boolalpha << validatePin(pin) << std::endl;
    }

    return 0;
}