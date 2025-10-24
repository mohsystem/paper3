
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

void checkPasswordStrength(const char* password, char* result) {
    if (password == NULL || strlen(password) == 0) {
        strcpy(result, "Weak: Password is null or empty");
        return;
    }
    
    bool hasMinLength = strlen(password) >= 8;
    bool hasUppercase = false;
    bool hasLowercase = false;
    bool hasDigit = false;
    
    for (int i = 0; i < strlen(password); i++) {
        if (isupper(password[i])) hasUppercase = true;
        if (islower(password[i])) hasLowercase = true;
        if (isdigit(password[i])) hasDigit = true;
    }
    
    if (hasMinLength && hasUppercase && hasLowercase && hasDigit) {
        strcpy(result, "Strong");
    } else {
        strcpy(result, "Weak: ");
        if (!hasMinLength) strcat(result, "Less than 8 characters. ");
        if (!hasUppercase) strcat(result, "No uppercase letter. ");
        if (!hasLowercase) strcat(result, "No lowercase letter. ");
        if (!hasDigit) strcat(result, "No digit. ");
    }
}

int main() {
    char result[200];
    
    // Test cases
    checkPasswordStrength("Pass123word", result);
    printf("Test 1: %s\\n", result);
    
    checkPasswordStrength("weak", result);
    printf("Test 2: %s\\n", result);
    
    checkPasswordStrength("NoDigits", result);
    printf("Test 3: %s\\n", result);
    
    checkPasswordStrength("alllowercase123", result);
    printf("Test 4: %s\\n", result);
    
    checkPasswordStrength("ALLUPPERCASE123", result);
    printf("Test 5: %s\\n", result);
    
    return 0;
}
