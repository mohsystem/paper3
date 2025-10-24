#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @brief Checks the strength of a given password.
 * 
 * @param password A constant character pointer to the password string.
 * @return true if the password is strong, false otherwise.
 */
bool checkPasswordStrength(const char *password) {
    // Handle NULL pointer
    if (password == NULL) {
        return false;
    }

    size_t len = strlen(password);
    if (len < 8) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (size_t i = 0; i < len; i++) {
        // Cast to unsigned char for safety with ctype functions
        unsigned char ch = password[i];
        if (isupper(ch)) {
            hasUpper = true;
        } else if (islower(ch)) {
            hasLower = true;
        } else if (isdigit(ch)) {
            hasDigit = true;
        }
    }

    return hasUpper && hasLower && hasDigit;
}

int main() {
    const char* testCases[] = {
        "StrongPass123", // Strong
        "weak",          // Weak: too short, no upper, no digit
        "NoDigitPass",   // Weak: no digit
        "nouppercase1",  // Weak: no uppercase
        "NOLOWERCASE1"   // Weak: no lowercase
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("C Test Cases:\n");
    for (int i = 0; i < numTestCases; i++) {
        printf("Password: \"%s\" -> Strong: %s\n", 
               testCases[i], 
               checkPasswordStrength(testCases[i]) ? "true" : "false");
    }

    return 0;
}