#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * Validates a PIN according to the rules:
 * - Must be exactly 4 or 6 digits long.
 * - Must contain only numeric digits.
 *
 * @param pin The PIN C-string to validate.
 * @return true if the PIN is valid, false otherwise.
 */
bool validatePin(const char *pin) {
    if (pin == NULL) {
        return false;
    }
    
    size_t len = strlen(pin);

    // Check if the length is 4 or 6
    if (len != 4 && len != 6) {
        return false;
    }

    // Check if all characters are digits
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)pin[i])) {
            return false;
        }
    }

    return true;
}

int main() {
    // Test cases
    const char* testPins[] = {
        "1234",    // true
        "12345",   // false
        "a234",    // false
        "123456",  // true
        "12.0"     // false
    };
    int num_tests = sizeof(testPins) / sizeof(testPins[0]);

    printf("C Test Cases:\n");
    for (int i = 0; i < num_tests; i++) {
        printf("\"%s\" --> %s\n", testPins[i], validatePin(testPins[i]) ? "true" : "false");
    }

    return 0;
}