
#include <stdio.h>
#include <string.h>
#include <ctype.h>

const char* checkPasswordStrength(const char* password) {
    if (password == NULL) {
        return "Weak";
    }
    
    int length = strlen(password);
    int hasMinLength = length >= 8;
    int hasUppercase = 0;
    int hasLowercase = 0;
    int hasDigit = 0;
    
    for (int i = 0; i < length; i++) {
        if (isupper(password[i])) {
            hasUppercase = 1;
        } else if (islower(password[i])) {
            hasLowercase = 1;
        } else if (isdigit(password[i])) {
            hasDigit = 1;
        }
    }
    
    if (hasMinLength && hasUppercase && hasLowercase && hasDigit) {
        return "Strong";
    } else {
        return "Weak";
    }
}

int main() {
    // Test case 1: Strong password
    printf("Test 1 - 'Password123': %s\\n", checkPasswordStrength("Password123"));
    
    // Test case 2: Too short
    printf("Test 2 - 'Pass1': %s\\n", checkPasswordStrength("Pass1"));
    
    // Test case 3: No uppercase
    printf("Test 3 - 'password123': %s\\n", checkPasswordStrength("password123"));
    
    // Test case 4: No digit
    printf("Test 4 - 'PasswordTest': %s\\n", checkPasswordStrength("PasswordTest"));
    
    // Test case 5: Strong password with special characters
    printf("Test 5 - 'SecurePass99!': %s\\n", checkPasswordStrength("SecurePass99!"));
    
    return 0;
}
