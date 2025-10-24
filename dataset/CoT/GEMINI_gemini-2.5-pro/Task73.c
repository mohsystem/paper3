#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/**
 * Checks if a given password is valid for an admin user.
 * A valid admin password must:
 * 1. Be at least 12 characters long.
 * 2. Contain at least one uppercase letter.
 * 3. Contain at least one lowercase letter.
 * 4. Contain at least one digit.
 * 5. Contain at least one special character from the set: !@#$%^&*()-_=+[]{}|;:'",.<>/?
 *
 * @param password The password string to validate.
 * @return true if the password is valid, false otherwise.
 */
bool isValidAdminPassword(const char* password) {
    // 1. Rule: Check for NULL pointer and minimum length
    if (password == NULL || strlen(password) < 12) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    const char* specialChars = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?";
    
    for (int i = 0; password[i] != '\0'; i++) {
        char c = password[i];
        if (isupper(c)) {
            hasUpper = true;
        } else if (islower(c)) {
            hasLower = true;
        } else if (isdigit(c)) {
            hasDigit = true;
        } else if (strchr(specialChars, c) != NULL) {
            hasSpecial = true;
        }
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}

int main() {
    const char* testPasswords[] = {
        "AdminP@ssw0rd123",  // Valid
        "short",              // Invalid: too short
        "NoDigitPassword!",   // Invalid: no digit
        "nouppercase@123",    // Invalid: no uppercase
        "NoSpecialChar123",   // Invalid: no special character
        NULL
    };
    
    printf("Running C Test Cases:\n");
    for (int i = 0; testPasswords[i] != NULL; ++i) {
        printf("Password: \"%s\" is valid: %s\n", 
               testPasswords[i], 
               isValidAdminPassword(testPasswords[i]) ? "true" : "false");
    }

    return 0;
}