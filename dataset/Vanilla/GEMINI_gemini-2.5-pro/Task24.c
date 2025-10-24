#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @brief Validates a PIN to be exactly 4 or 6 digits.
 * @param pin The PIN string to validate.
 * @return true if the PIN is valid, false otherwise.
 */
bool validatePin(const char* pin) {
    size_t len = strlen(pin);
    if (len != 4 && len != 6) {
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)pin[i])) {
            return false;
        }
    }
    return true;
}

int main() {
    // 5 test cases
    const char* testCases[] = {"1234", "12345", "a234", "123456", "123"};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        printf("\"%s\"   -->  %s\n", testCases[i], validatePin(testCases[i]) ? "true" : "false");
    }

    return 0;
}