#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * @brief Case-insensitive substring search.
 *
 * @param haystack The string to search in.
 * @param needle The string to search for.
 * @return A pointer to the beginning of the substring, or NULL if not found.
 */
const char* find_str_case_insensitive(const char* haystack, const char* needle) {
    if (!needle || !*needle) return haystack;

    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++;
            n++;
        }
        if (!*n) {
            return haystack; // Found
        }
        haystack++;
    }
    return NULL;
}

/**
 * @brief Checks if a given password is valid for an admin user.
 * 
 * A valid admin password must:
 * 1. Be at least 10 characters long.
 * 2. Contain at least one uppercase letter.
 * 3. Contain at least one lowercase letter.
 * 4. Contain at least one digit.
 * 5. Contain at least one special character from the set !@#$%^&*()-_=+[]{}|;:'",.<>/?
 * 6. Contain the substring "admin" (case-insensitive).
 * 
 * @param password The password string to validate.
 * @return true if the password is valid, false otherwise.
 */
bool isValidAdminPassword(const char* password) {
    if (password == NULL || strlen(password) < 10) {
        return false;
    }

    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_special = false;
    
    const char* special_chars = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?";
    
    for (int i = 0; password[i] != '\0'; i++) {
        unsigned char c = password[i];
        if (isupper(c)) {
            has_upper = true;
        } else if (islower(c)) {
            has_lower = true;
        } else if (isdigit(c)) {
            has_digit = true;
        } else if (strchr(special_chars, c) != NULL) {
            has_special = true;
        }
    }

    // Case-insensitive check for "admin"
    bool has_admin = (find_str_case_insensitive(password, "admin") != NULL);

    return has_upper && has_lower && has_digit && has_special && has_admin;
}

int main() {
    const char* testPasswords[] = {
        "ValidAdminPass123!",   // Should be true
        "shortAd1!",              // Should be false (too short)
        "nouppercaseadmin1!",     // Should be false (no uppercase)
        "NoSpecialAdmin123",      // Should be false (no special character)
        "NoKeyword123!@#",        // Should be false (no "admin" substring)
        "NoDigitsADMIN!@#"        // Should be false (no digits)
    };
    int num_tests = sizeof(testPasswords) / sizeof(testPasswords[0]);
    
    printf("Running C Test Cases:\n");
    for (int i = 0; i < num_tests; i++) {
        printf("Password: \"%s\" is valid? %s\n", 
               testPasswords[i], 
               isValidAdminPassword(testPasswords[i]) ? "true" : "false");
    }

    return 0;
}