
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Password validation function for admin users
// Requirements: min 12 chars, at least one uppercase, lowercase, digit, special char
bool isValidAdminPassword(const char* password) {
    // Security: Input validation - check for NULL pointer
    if (password == NULL) {
        return false;
    }
    
    // Get password length with bounds check
    size_t len = strnlen(password, 129); // Check up to max + 1
    
    // Security: Length validation (min 12, max 128)
    // Prevents too weak passwords and DoS attacks from excessive length
    if (len < 12 || len > 128) {
        return false;
    }
    
    // Security: Check for embedded null bytes (truncation attack)
    for (size_t i = 0; i < len; i++) {
        if (password[i] == '\\0') {
            return false;
        }
    }
    
    // Check for required character types
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    // Validate each character and accumulate requirements
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        
        // Security: Only allow printable ASCII (prevents encoding attacks)
        if (c < 32 || c > 126) {
            return false;
        }
        
        if (isupper(c)) hasUpper = true;
        else if (islower(c)) hasLower = true;
        else if (isdigit(c)) hasDigit = true;
        else if (ispunct(c)) hasSpecial = true;
    }
    
    // Require all four character types for admin-level security
    if (!hasUpper || !hasLower || !hasDigit || !hasSpecial) {
        return false;
    }
    
    // Security: Detect sequential characters (abc, 123)
    int sequentialCount = 0;
    for (size_t i = 0; i < len - 1; i++) {
        if (password[i] + 1 == password[i + 1]) {
            sequentialCount++;
            if (sequentialCount >= 3) {
                return false;
            }
        } else {
            sequentialCount = 0;
        }
    }
    
    // Security: Detect repeated characters (aaa, 111)
    int repeatCount = 1;
    for (size_t i = 0; i < len - 1; i++) {
        if (password[i] == password[i + 1]) {
            repeatCount++;
            if (repeatCount >= 3) {
                return false;
            }
        } else {
            repeatCount = 1;
        }
    }
    
    // Security: Convert to lowercase for case-insensitive pattern checking
    char lowerPassword[129]; // max 128 + null terminator
    for (size_t i = 0; i < len && i < 128; i++) {
        lowerPassword[i] = (char)tolower((unsigned char)password[i]);
    }
    lowerPassword[len] = '\\0';
    
    // Security: Check against common weak passwords
    const char* weakPasswords[] = {
        "password123!", "admin123456!", "administrator1!", 
        "welcome123!", "qwerty123456!"
    };
    
    for (size_t i = 0; i < 5; i++) {
        if (strcmp(lowerPassword, weakPasswords[i]) == 0) {
            return false;
        }
    }
    
    // Security: Check for dictionary words within password
    const char* commonWords[] = {
        "password", "admin", "administrator", "welcome", 
        "qwerty", "letmein", "monkey", "dragon"
    };
    
    for (size_t i = 0; i < 8; i++) {
        if (strstr(lowerPassword, commonWords[i]) != NULL) {
            return false;
        }
    }
    
    return true;
}

int main(void) {
    // Test case 1: Valid strong password
    const char* test1 = "Adm!n2024Secure#";
    printf("Test 1 - '%s': %s\\n", test1, 
           isValidAdminPassword(test1) ? "VALID" : "INVALID");
    
    // Test case 2: Too short (less than 12 characters)
    const char* test2 = "Short1!";
    printf("Test 2 - '%s': %s\\n", test2, 
           isValidAdminPassword(test2) ? "VALID" : "INVALID");
    
    // Test case 3: Missing special character
    const char* test3 = "NoSpecialChar123";
    printf("Test 3 - '%s': %s\\n", test3, 
           isValidAdminPassword(test3) ? "VALID" : "INVALID");
    
    // Test case 4: Contains common weak pattern
    const char* test4 = "Password123!";
    printf("Test 4 - '%s': %s\\n", test4, 
           isValidAdminPassword(test4) ? "VALID" : "INVALID");
    
    // Test case 5: Valid complex password
    const char* test5 = "Xk9$mPz7&Qw3!Rt";
    printf("Test 5 - '%s': %s\\n", test5, 
           isValidAdminPassword(test5) ? "VALID" : "INVALID");
    
    return 0;
}
