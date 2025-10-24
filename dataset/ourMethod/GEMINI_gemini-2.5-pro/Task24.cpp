#include <iostream>
#include <string>
#include <cctype>
#include <vector>

// Function to validate an ATM PIN
bool validatePIN(const std::string& pin) {
    size_t len = pin.length();

    // Rule: PIN must be exactly 4 or 6 digits long.
    if (len != 4 && len != 6) {
        return false;
    }

    // Rule: PIN must contain only digits.
    for (char c : pin) {
        // Use isdigit with a cast to unsigned char for safety with all char ranges.
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    return true;
}

int main() {
    std::vector<std::string> test_cases = {
        "1234",   // Expected: true
        "12345",  // Expected: false (invalid length)
        "a234",   // Expected: false (contains non-digit)
        "123456", // Expected: true
        "-123"    // Expected: false (contains non-digit)
    };

    std::cout << std::boolalpha;
    for (const auto& test_case : test_cases) {
        std::cout << "PIN: \"" << test_case << "\" -> Valid: " << validatePIN(test_case) << std::endl;
    }

    return 0;
}