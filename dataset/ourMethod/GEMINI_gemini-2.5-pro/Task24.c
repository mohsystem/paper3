#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Function to validate an ATM PIN
bool validatePIN(const char* pin) {
    // Rule #1: Treat all inputs as untrusted. Check for NULL pointer.
    if (pin == NULL) {
        return false;
    }

    size_t len = strlen(pin);

    // Rule: PIN must be exactly 4 or 6 digits long.
    if (len != 4 && len != 6) {
        return false;
    }

    // Rule: PIN must contain only digits.
    for (size_t i = 0; i < len; ++i) {
        // Use isdigit with a cast to unsigned char for safety with all char ranges.
        if (!isdigit((unsigned char)pin[i])) {
            return false;
        }
    }

    return true;
}

int main() {
    const char* test_cases[] = {
        "1234",   // Expected: true
        "12345",  // Expected: false (invalid length)
        "a234",   // Expected: false (contains non-digit)
        "123456", // Expected: true
        "123 "    // Expected: false (contains non-digit)
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        printf("PIN: \"%s\" -> Valid: %s\n", test_cases[i], validatePIN(test_cases[i]) ? "true" : "false");
    }

    return 0;
}