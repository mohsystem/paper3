
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool checkPasswordStrength(const char* password) {
    // Handle NULL or empty password securely
    if (password == NULL || password[0] == '\\0') {
        return false;
    }
    
    size_t length = strlen(password);
    
    // Check minimum length
    if (length < 8) {
        return false;
    }
    
    bool hasUppercase = false;
    bool hasLowercase = false;
    bool hasDigit = false;
    
    // Iterate through each character to check criteria
    for (size_t i = 0; i < length; i++) {
        unsigned char c = (unsigned char)password[i];
        
        if (isupper(c)) {
            hasUppercase = true;
        } else if (islower(c)) {
            hasLowercase = true;
        } else if (isdigit(c)) {
            hasDigit = true;
        }
        
        // Early exit if all criteria met
        if (hasUppercase && hasLowercase && hasDigit) {
            return true;
        }
    }
    
    return hasUppercase && hasLowercase && hasDigit;
}

int main() {
    // Test cases
    const char* testPasswords[] = {
        "Abcd1234",      // Valid - meets all criteria
        "abcd1234",      // Invalid - no uppercase
        "ABCD1234",      // Invalid - no lowercase
        "Abcdefgh",      // Invalid - no digit
        "Abc123"         // Invalid - too short
    };
    
    int numTests = sizeof(testPasswords) / sizeof(testPasswords[0]);
    
    printf("Password Strength Checker Results:\\n");
    
    for (int i = 0; i < numTests; i++) {
        bool isStrong = checkPasswordStrength(testPasswords[i]);
        printf("Password: %s -> %s\\n", testPasswords[i], 
               isStrong ? "STRONG" : "WEAK");
    }
    
    return 0;
}
