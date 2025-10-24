
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool isValidAdminPassword(const char* password) {
    if (password == NULL || strlen(password) < 12) {
        return false;
    }
    
    bool hasUppercase = false;
    bool hasLowercase = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    const char* specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    for (size_t i = 0; i < strlen(password); i++) {
        char c = password[i];
        if (isupper(c)) {
            hasUppercase = true;
        } else if (islower(c)) {
            hasLowercase = true;
        } else if (isdigit(c)) {
            hasDigit = true;
        } else if (strchr(specialChars, c) != NULL) {
            hasSpecial = true;
        }
    }
    
    // Check for common weak passwords
    char lowerPassword[256];
    strncpy(lowerPassword, password, sizeof(lowerPassword) - 1);
    lowerPassword[sizeof(lowerPassword) - 1] = '\\0';
    
    for (size_t i = 0; i < strlen(lowerPassword); i++) {
        lowerPassword[i] = tolower(lowerPassword[i]);
    }
    
    const char* weakPasswords[] = {"password", "admin", "123456", "qwerty", "letmein"};
    int numWeak = 5;
    
    for (int i = 0; i < numWeak; i++) {
        if (strstr(lowerPassword, weakPasswords[i]) != NULL) {
            return false;
        }
    }
    
    return hasUppercase && hasLowercase && hasDigit && hasSpecial;
}

int main() {
    // Test cases
    const char* testPasswords[] = {
        "Admin@123456",           // Valid
        "SecureP@ssw0rd!",       // Valid
        "weak",                   // Invalid - too short
        "NoSpecialChar123",       // Invalid - no special character
        "Admin@Password2024"      // Valid
    };
    
    int numTests = 5;
    
    printf("Admin Password Validation Results:\\n");
    printf("=====================================\\n");
    
    for (int i = 0; i < numTests; i++) {
        bool isValid = isValidAdminPassword(testPasswords[i]);
        printf("Password: %s -> %s\\n", testPasswords[i], isValid ? "VALID" : "INVALID");
    }
    
    return 0;
}
