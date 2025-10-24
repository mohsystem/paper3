#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @brief Checks if a password meets the strength criteria.
 * 
 * Criteria:
 * - At least 8 characters long
 * - Contains at least one uppercase letter
 * - Contains at least one lowercase letter
 * - Contains at least one digit
 * 
 * @param password A pointer to the null-terminated string to check.
 * @return true if the password is strong, false otherwise.
 */
bool isPasswordStrong(const char* password) {
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
        // Cast to unsigned char to prevent undefined behavior with is... functions
        unsigned char ch = (unsigned char)password[i];
        if (isupper(ch)) {
            hasUpper = true;
        } else if (islower(ch)) {
            hasLower = true;
        } else if (isdigit(ch)) {
            hasDigit = true;
        }

        // Optimization: if all conditions are met, no need to check further.
        if (hasUpper && hasLower && hasDigit) {
            return true;
        }
    }

    return hasUpper && hasLower && hasDigit;
}

int main() {
    const char* testPasswords[] = {
        "StrongPwd1",      // Should be true
        "weakpwd1",        // Should be false (missing uppercase)
        "WEAKPWD1",        // Should be false (missing lowercase)
        "WeakPassword",    // Should be false (missing digit)
        "Wpwd1"            // Should be false (too short)
    };
    int num_tests = sizeof(testPasswords) / sizeof(testPasswords[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* password = testPasswords[i];
        printf("Password: \"%s\" -> Strong: %s\n", 
               password, isPasswordStrong(password) ? "true" : "false");
    }

    return 0;
}