
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>

/* Validates email format using POSIX regex
 * Security: Checks length bounds and uses strict pattern
 */
bool validateEmail(const char* email) {
    regex_t regex;
    int result;
    
    // Security: Validate input is not NULL
    if (email == NULL) {
        return false;
    }
    
    // Security: Check length to prevent DoS
    size_t len = strnlen(email, 255);
    if (len == 0 || len > 254) {
        return false;
    }
    
    // Compile regex pattern - strict email format
    result = regcomp(&regex, 
        "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]\\\\{2,\\\\}$",
        REG_EXTENDED | REG_NOSUB);
    
    if (result != 0) {
        return false;
    }
    
    // Execute regex match
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return (result == 0);
}

/* Validates phone number (US format)
 * Security: Enforces length limits and strict format
 */
bool validatePhone(const char* phone) {
    regex_t regex;
    int result;
    
    // Security: NULL check
    if (phone == NULL) {
        return false;
    }
    
    // Security: Length validation
    size_t len = strnlen(phone, 21);
    if (len == 0 || len > 20) {
        return false;
    }
    
    // Pattern for various phone formats
    result = regcomp(&regex,
        "^(\\\\([0-9]\\\\{3\\\\}\\\\) |[0-9]\\\\{3\\\\}-)?[0-9]\\\\{3\\\\}-\\\\?[0-9]\\\\{4\\\\}$",
        REG_EXTENDED | REG_NOSUB);
    
    if (result != 0) {
        return false;
    }
    
    result = regexec(&regex, phone, 0, NULL, 0);
    regfree(&regex);
    
    return (result == 0);
}

/* Validates username
 * Security: Alphanumeric only with limited special characters
 */
bool validateUsername(const char* username) {
    regex_t regex;
    int result;
    
    // Security: NULL check
    if (username == NULL) {
        return false;
    }
    
    // Security: Enforce reasonable length limits
    size_t len = strnlen(username, 21);
    if (len < 3 || len > 20) {
        return false;
    }
    
    // Only alphanumeric, underscore, hyphen
    result = regcomp(&regex,
        "^[a-zA-Z0-9_-]\\\\{3,20\\\\}$",
        REG_EXTENDED | REG_NOSUB);
    
    if (result != 0) {
        return false;
    }
    
    result = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return (result == 0);
}

/* Validates password strength
 * Security: Enforces complexity requirements
 */
bool validatePassword(const char* password) {
    regex_t upper, lower, digit, special;
    int result;
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    
    // Security: NULL check
    if (password == NULL) {
        return false;
    }
    
    // Security: Length bounds
    size_t len = strnlen(password, 129);
    if (len < 8 || len > 128) {
        return false;
    }
    
    // Check for uppercase
    result = regcomp(&upper, "[A-Z]", REG_EXTENDED | REG_NOSUB);
    if (result == 0) {
        hasUpper = (regexec(&upper, password, 0, NULL, 0) == 0);
        regfree(&upper);
    }
    
    // Check for lowercase
    result = regcomp(&lower, "[a-z]", REG_EXTENDED | REG_NOSUB);
    if (result == 0) {
        hasLower = (regexec(&lower, password, 0, NULL, 0) == 0);
        regfree(&lower);
    }
    
    // Check for digit
    result = regcomp(&digit, "[0-9]", REG_EXTENDED | REG_NOSUB);
    if (result == 0) {
        hasDigit = (regexec(&digit, password, 0, NULL, 0) == 0);
        regfree(&digit);
    }
    
    // Check for special character
    result = regcomp(&special, "[!@#$%^&*(),.?\\":{}|<>]", REG_EXTENDED | REG_NOSUB);
    if (result == 0) {
        hasSpecial = (regexec(&special, password, 0, NULL, 0) == 0);
        regfree(&special);
    }
    
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

/* Validates IPv4 address
 * Security: Strict format validation
 */
bool validateIPv4(const char* ip) {
    regex_t regex;
    int result;
    
    // Security: NULL check
    if (ip == NULL) {
        return false;
    }
    
    // Security: Length check
    size_t len = strnlen(ip, 16);
    if (len == 0 || len > 15) {
        return false;
    }
    
    // Pattern with proper octet boundaries
    result = regcomp(&regex,
        "^((25[0-5]|2[0-4][0-9]|[01]\\\\?[0-9][0-9]\\\\?)\\\\.)"
        "\\\\{3\\\\}(25[0-5]|2[0-4][0-9]|[01]\\\\?[0-9][0-9]\\\\?)$",
        REG_EXTENDED | REG_NOSUB);
    
    if (result != 0) {
        return false;
    }
    
    result = regexec(&regex, ip, 0, NULL, 0);
    regfree(&regex);
    
    return (result == 0);
}

int main(void) {
    // Test case 1: Email validation
    printf("Test 1 - Email Validation:\\n");
    const char* email1 = "user@example.com";
    const char* email2 = "invalid.email";
    printf("  %s: %s\\n", email1, validateEmail(email1) ? "Valid" : "Invalid");
    printf("  %s: %s\\n", email2, validateEmail(email2) ? "Valid" : "Invalid");
    
    // Test case 2: Phone validation
    printf("\\nTest 2 - Phone Validation:\\n");
    const char* phone1 = "(123) 456-7890";
    const char* phone2 = "123-456-7890";
    const char* phone3 = "invalid-phone";
    printf("  %s: %s\\n", phone1, validatePhone(phone1) ? "Valid" : "Invalid");
    printf("  %s: %s\\n", phone2, validatePhone(phone2) ? "Valid" : "Invalid");
    printf("  %s: %s\\n", phone3, validatePhone(phone3) ? "Valid" : "Invalid");
    
    // Test case 3: Username validation
    printf("\\nTest 3 - Username Validation:\\n");
    const char* user1 = "john_doe123";
    const char* user2 = "ab";
    const char* user3 = "user@name";
    printf("  %s: %s\\n", user1, validateUsername(user1) ? "Valid" : "Invalid");
    printf("  %s: %s\\n", user2, validateUsername(user2) ? "Valid" : "Invalid");
    printf("  %s: %s\\n", user3, validateUsername(user3) ? "Valid" : "Invalid");
    
    // Test case 4: Password validation
    printf("\\nTest 4 - Password Validation:\\n");
    const char* pass1 = "Strong@Pass123";
    const char* pass2 = "weakpass";
    printf("  Strong@Pass123: %s\\n", validatePassword(pass1) ? "Valid" : "Invalid");
    printf("  weakpass: %s\\n", validatePassword(pass2) ? "Valid" : "Invalid");
    
    // Test case 5: IP address validation
    printf("\\nTest 5 - IPv4 Validation:\\n");
    const char* ip1 = "192.168.1.1";
    const char* ip2 = "256.1.1.1";
    const char* ip3 = "10.0.0.1";
    printf("  %s: %s\\n", ip1, validateIPv4(ip1) ? "Valid" : "Invalid");
    printf("  %s: %s\\n", ip2, validateIPv4(ip2) ? "Valid" : "Invalid");
    printf("  %s: %s\\n", ip3, validateIPv4(ip3) ? "Valid" : "Invalid");
    
    return 0;
}
