
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool isValidPassword(const char* password) {
    if (password == NULL || strlen(password) < 8) {
        return false;
    }
    
    bool hasUpperCase = false;
    bool hasLowerCase = false;
    bool hasDigit = false;
    bool hasSpecialChar = false;
    
    const char* specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    for (int i = 0; password[i] != '\\0'; i++) {
        char c = password[i];
        if (isupper(c)) {
            hasUpperCase = true;
        } else if (islower(c)) {
            hasLowerCase = true;
        } else if (isdigit(c)) {
            hasDigit = true;
        } else if (strchr(specialChars, c) != NULL) {
            hasSpecialChar = true;
        }
    }
    
    return hasUpperCase && hasLowerCase && hasDigit && hasSpecialChar;
}

int main() {
    // Test cases
    printf("Test 1: 'Admin@123' -> %s\\n", isValidPassword("Admin@123") ? "true" : "false");
    printf("Test 2: 'password' -> %s\\n", isValidPassword("password") ? "true" : "false");
    printf("Test 3: 'Pass123' -> %s\\n", isValidPassword("Pass123") ? "true" : "false");
    printf("Test 4: 'ADMIN@123' -> %s\\n", isValidPassword("ADMIN@123") ? "true" : "false");
    printf("Test 5: 'Secure#Pass2024' -> %s\\n", isValidPassword("Secure#Pass2024") ? "true" : "false");
    
    return 0;
}
