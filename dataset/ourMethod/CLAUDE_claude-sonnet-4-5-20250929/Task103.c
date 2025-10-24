
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Function to check password strength
// Returns: 0 = weak, 1 = strong
// Security: Input validation, no logging of password, bounds checking
int checkPasswordStrength(const char* password) {
    // Input validation: check for NULL pointer
    // Security: Prevents null pointer dereference
    if (password == NULL) {
        return 0;
    }
    
    // Get length with bounds check
    // Security: Use strnlen_s if available, otherwise manual check
    size_t length = 0;
    const size_t MAX_PASSWORD_LENGTH = 128;
    
    // Calculate length with maximum bound
    // Security: Prevents unbounded string operations
    while (length < MAX_PASSWORD_LENGTH && password[length] != '\\0') {
        length++;
    }
    
    // Check if password exceeds maximum length
    // Security: Prevents potential DoS through extremely long inputs
    if (length >= MAX_PASSWORD_LENGTH && password[MAX_PASSWORD_LENGTH] != '\\0') {
        return 0;
    }
    
    // Check minimum length (at least 8 characters)
    // Security: Enforces strong password policy
    if (length < 8) {
        return 0;
    }
    
    // Initialize flags for character type checks
    int hasUpper = 0;
    int hasLower = 0;
    int hasDigit = 0;
    
    // Iterate through all characters to check requirements
    // Security: Process all characters to avoid timing attacks
    for (size_t i = 0; i < length; i++) {
        unsigned char c = (unsigned char)password[i];
        
        // Check for uppercase letter
        if (isupper(c)) {
            hasUpper = 1;
        }
        // Check for lowercase letter
        if (islower(c)) {
            hasLower = 1;
        }
        // Check for digit
        if (isdigit(c)) {
            hasDigit = 1;
        }
    }
    
    // Return 1 if all criteria are met, 0 otherwise
    // Security: Simple boolean logic prevents timing-based leaks
    return (hasUpper && hasLower && hasDigit) ? 1 : 0;
}

int main(void) {
    // Test cases: demonstrate various password strengths
    // Security: Test passwords are not real credentials
    const char* testPasswords[5] = {
        "Weak1",           // Too short (5 chars)
        "weakpassword1",   // No uppercase
        "WEAKPASSWORD1",   // No lowercase
        "WeakPassword",    // No digit
        "Strong1Pass"      // Valid: 12 chars, upper, lower, digit
    };
    
    printf("Password Strength Checker\\n");
    printf("=========================\\n\\n");
    
    // Test each password
    for (int i = 0; i < 5; i++) {
        int result = checkPasswordStrength(testPasswords[i]);
        
        // Security: Do not log the actual password in production
        printf("Test %d: %s\\n", i + 1, result == 1 ? "STRONG" : "WEAK");
        
        // Display criteria status
        size_t length = 0;
        int hasUpper = 0, hasLower = 0, hasDigit = 0;
        
        // Recalculate for display purposes with bounds check
        while (length < 128 && testPasswords[i][length] != '\\0') {
            unsigned char c = (unsigned char)testPasswords[i][length];
            if (isupper(c)) hasUpper = 1;
            if (islower(c)) hasLower = 1;
            if (isdigit(c)) hasDigit = 1;
            length++;
        }
        
        printf("  Length >= 8: %s\\n", length >= 8 ? "YES" : "NO");
        printf("  Has uppercase: %s\\n", hasUpper ? "YES" : "NO");
        printf("  Has lowercase: %s\\n", hasLower ? "YES" : "NO");
        printf("  Has digit: %s\\n\\n", hasDigit ? "YES" : "NO");
    }
    
    return 0;
}
