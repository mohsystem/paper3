#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * @brief Checks the strength of a given password.
 * 
 * A strong password must be at least 8 characters long, contain both
 * uppercase and lowercase letters, and at least one digit.
 * 
 * @param password A constant character pointer to the password string.
 * @return true if the password is strong, false otherwise.
 */
bool checkPasswordStrength(const char* password) {
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

    for (size_t i = 0; i < len; ++i) {
        char c = password[i];
        if (isupper((unsigned char)c)) {
            hasUpper = true;
        } else if (islower((unsigned char)c)) {
            hasLower = true;
        } else if (isdigit((unsigned char)c)) {
            hasDigit = true;
        }
    }

    return hasUpper && hasLower && hasDigit;
}

int main() {
    const char* testCases[] = {
        "Password123",    // Strong
        "password123",    // Weak (no uppercase)
        "PASSWORD123",    // Weak (no lowercase)
        "Password",       // Weak (no digit)
        "pass"            // Weak (too short)
    };
    
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < numTestCases; ++i) {
        bool isStrong = checkPasswordStrength(testCases[i]);
        printf("Password: \"%s\" -> Strong: %s\n", testCases[i], isStrong ? "true" : "false");
    }

    return 0;
}