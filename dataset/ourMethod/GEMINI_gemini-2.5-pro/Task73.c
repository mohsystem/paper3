#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MIN_LENGTH 12
#define MAX_LENGTH 128

const char* SPECIAL_CHARACTERS = "!@#$%^&*()_+-=[]{};':\"\\|,.<>/?~";

/**
 * @brief Checks if a given password is valid for an admin user.
 * 
 * Policy:
 * - Length between 12 and 128 characters.
 * - Contains at least one uppercase letter.
 * - Contains at least one lowercase letter.
 * - Contains at least one digit.
 * - Contains at least one special character.
 * 
 * @param password The password string to validate.
 * @return true if the password is valid, false otherwise.
 */
bool isAdminPasswordValid(const char* password) {
    if (password == NULL) {
        return false;
    }

    size_t len = strlen(password);
    if (len < MIN_LENGTH || len > MAX_LENGTH) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (size_t i = 0; i < len; ++i) {
        char ch = password[i];
        if (isupper((unsigned char)ch)) {
            hasUpper = true;
        } else if (islower((unsigned char)ch)) {
            hasLower = true;
        } else if (isdigit((unsigned char)ch)) {
            hasDigit = true;
        } else if (strchr(SPECIAL_CHARACTERS, ch) != NULL) {
            hasSpecial = true;
        }
        
        // Optimization: if all conditions are met, we can exit early.
        if (hasUpper && hasLower && hasDigit && hasSpecial) {
            return true;
        }
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}

int main() {
    const char* testPasswords[] = {
        "Admin@1234567",       // Valid
        "admin@1234567",       // Invalid: no uppercase
        "Admin1234567",        // Invalid: no special character
        "Admin@",              // Invalid: too short
        "ADMIN@1234567"        // Invalid: no lowercase
    };
    int numTests = sizeof(testPasswords) / sizeof(testPasswords[0]);

    for (int i = 0; i < numTests; ++i) {
        printf("Password: \"%s\" is valid: %s\n", 
               testPasswords[i], 
               isAdminPasswordValid(testPasswords[i]) ? "true" : "false");
    }

    return 0;
}