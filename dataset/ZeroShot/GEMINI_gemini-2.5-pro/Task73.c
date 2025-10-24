#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MIN_PASSWORD_LENGTH 12

// Forward declaration for the case-insensitive string search function
char* strcasestr(const char* haystack, const char* needle);

/**
 * Checks if a given password is valid for an admin user based on security policies.
 *
 * @param password The password string to validate.
 * @return true if the password is valid, false otherwise.
 */
bool isAdminPasswordValid(const char* password) {
    if (password == NULL || strlen(password) < MIN_PASSWORD_LENGTH) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    for (int i = 0; password[i] != '\0'; i++) {
        if (isupper(password[i])) hasUpper = true;
        else if (islower(password[i])) hasLower = true;
        else if (isdigit(password[i])) hasDigit = true;
        else if (ispunct(password[i]) || strchr("~`!@#$%^&*()_+-=[]{}|;':\",./<>?", password[i])) {
            hasSpecial = true;
        }
    }

    if (!hasUpper || !hasLower || !hasDigit || !hasSpecial) {
        return false;
    }

    const char* commonPasswords[] = {
        "password", "123456", "123456789", "qwerty", "12345", "12345678", 
        "111111", "1234567", "password123", NULL
    };
    for (int i = 0; commonPasswords[i] != NULL; i++) {
        if (strcmp(password, commonPasswords[i]) == 0) {
            return false;
        }
    }

    // Check if the password contains "admin" (case-insensitive)
    if (strcasestr(password, "admin") != NULL) {
        return false;
    }

    return true;
}

/**
 * A simple implementation of strcasestr for platforms that don't have it (like MSVC).
 */
char* strcasestr(const char* haystack, const char* needle) {
    if (!needle || !*needle) {
        return (char*)haystack;
    }
    for (; *haystack; ++haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (tolower((unsigned char)*h) == tolower((unsigned char)*n) && *n) {
            h++;
            n++;
        }
        if (!*n) {
            return (char*)haystack;
        }
    }
    return NULL;
}

int main() {
    const char* testPasswords[] = {
        "ValidP@ssw0rd!2024",  // Valid
        "Sh0rt!P@s",           // Invalid: Too short
        "NoDigitPassword!",    // Invalid: Missing digit
        "password123",         // Invalid: Common password
        "MyAdm1nPassword!"     // Invalid: Contains "admin"
    };
    
    int numTests = sizeof(testPasswords) / sizeof(testPasswords[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < numTests; i++) {
        printf("Password: \"%s\" -> Is Valid: %s\n", 
               testPasswords[i], 
               isAdminPasswordValid(testPasswords[i]) ? "true" : "false");
    }

    return 0;
}