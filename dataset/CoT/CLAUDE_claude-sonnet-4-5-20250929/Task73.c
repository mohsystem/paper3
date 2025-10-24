
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MIN_PASSWORD_LENGTH 12
#define MAX_PASSWORD_LENGTH 256

bool isCommonPassword(const char* password) {
    const char* commonPasswords[] = {
        "password123!", "admin@123456", "welcome@2024", 
        "p@ssw0rd1234", "administrator1!"
    };
    int numCommon = 5;
    
    char lowerPassword[MAX_PASSWORD_LENGTH];
    int len = strlen(password);
    
    // Convert to lowercase for comparison
    for (int i = 0; i < len && i < MAX_PASSWORD_LENGTH - 1; i++) {
        lowerPassword[i] = tolower((unsigned char)password[i]);
    }
    lowerPassword[len] = '\\0';
    
    for (int i = 0; i < numCommon; i++) {
        if (strcmp(lowerPassword, commonPasswords[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

bool isValidAdminPassword(const char* password) {
    // Check for NULL or empty
    if (password == NULL || password[0] == '\\0') {
        return false;
    }
    
    // Check minimum length
    size_t length = strlen(password);
    if (length < MIN_PASSWORD_LENGTH || length >= MAX_PASSWORD_LENGTH) {
        return false;
    }
    
    // Check for common passwords
    if (isCommonPassword(password)) {
        return false;
    }
    
    // Check for required character types
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    for (size_t i = 0; i < length; i++) {
        unsigned char c = (unsigned char)password[i];
        
        if (isupper(c)) {
            hasUpper = true;
        } else if (islower(c)) {
            hasLower = true;
        } else if (isdigit(c)) {
            hasDigit = true;
        } else if (!isspace(c)) {
            hasSpecial = true;
        }
    }
    
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

int main() {
    // Test cases
    const char* testPasswords[] = {
        "SecureP@ssw0rd123",  // Valid
        "Weak1!",              // Too short
        "nosymbolsupper1",     // No uppercase or special
        "NOLOWERCASE123!",     // No lowercase
        "Admin@123456"         // Common password
    };
    
    printf("Admin Password Validation Results:\\n");
    for (int i = 0; i < 5; i++) {
        bool result = isValidAdminPassword(testPasswords[i]);
        printf("Test %d: \\"%s\\" -> %s\\n", i + 1, testPasswords[i], 
               result ? "VALID" : "INVALID");
    }
    
    return 0;
}
