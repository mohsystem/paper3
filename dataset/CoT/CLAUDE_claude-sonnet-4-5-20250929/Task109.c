
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

// Validates email format
bool validateEmail(const char* email) {
    if (email == NULL || strlen(email) == 0 || strlen(email) > 254) {
        return false;
    }
    regex_t regex;
    int result;
    
    // RFC 5322 compliant email pattern
    const char* pattern = "^[a-zA-Z0-9_+&*-]+(\\\\.[a-zA-Z0-9_+&*-]+)*@([a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$";
    
    result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result != 0) {
        return false;
    }
    
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

// Validates phone number (US format)
bool validatePhoneNumber(const char* phone) {
    if (phone == NULL || strlen(phone) == 0) {
        return false;
    }
    regex_t regex;
    int result;
    
    const char* pattern = "^(\\\\+?1)?[[:space:]-]?\\\\(?[0-9]{3}\\\\)?[[:space:]-]?[0-9]{3}[[:space:]-]?[0-9]{4}$";
    
    result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result != 0) {
        return false;
    }
    
    result = regexec(&regex, phone, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

// Validates URL format
bool validateURL(const char* url) {
    if (url == NULL || strlen(url) == 0 || strlen(url) > 2048) {
        return false;
    }
    regex_t regex;
    int result;
    
    const char* pattern = "^(https?://)(www\\\\.)?[-a-zA-Z0-9@:%._\\\\+~#=]{1,256}\\\\.[a-zA-Z0-9()]{1,6}([-a-zA-Z0-9()@:%_\\\\+.~#?&//=]*)$";
    
    result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result != 0) {
        return false;
    }
    
    result = regexec(&regex, url, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

// Validates strong password
bool validatePassword(const char* password) {
    if (password == NULL || strlen(password) < 8 || strlen(password) > 128) {
        return false;
    }
    
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    
    for (int i = 0; password[i] != '\\0'; i++) {
        if (password[i] >= 'a' && password[i] <= 'z') hasLower = true;
        else if (password[i] >= 'A' && password[i] <= 'Z') hasUpper = true;
        else if (password[i] >= '0' && password[i] <= '9') hasDigit = true;
        else if (strchr("@$!%*?&", password[i]) != NULL) hasSpecial = true;
    }
    
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

// Validates username (alphanumeric and underscore, 3-20 characters)
bool validateUsername(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return false;
    }
    regex_t regex;
    int result;
    
    const char* pattern = "^[a-zA-Z0-9_]{3,20}$";
    
    result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result != 0) {
        return false;
    }
    
    result = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

int main() {
    printf("=== User Input Validation Tests ===\\n\\n");
    
    // Test Case 1: Valid inputs
    printf("Test Case 1: Valid Inputs\\n");
    printf("Email 'user@example.com': %s\\n", validateEmail("user@example.com") ? "true" : "false");
    printf("Phone '123-456-7890': %s\\n", validatePhoneNumber("123-456-7890") ? "true" : "false");
    printf("URL 'https://www.example.com': %s\\n", validateURL("https://www.example.com") ? "true" : "false");
    printf("Password 'SecurePass123!': %s\\n", validatePassword("SecurePass123!") ? "true" : "false");
    printf("Username 'john_doe123': %s\\n", validateUsername("john_doe123") ? "true" : "false");
    printf("\\n");
    
    // Test Case 2: Invalid email
    printf("Test Case 2: Invalid Email\\n");
    printf("Email 'invalid.email': %s\\n", validateEmail("invalid.email") ? "true" : "false");
    printf("Email 'user@': %s\\n", validateEmail("user@") ? "true" : "false");
    printf("Email '@example.com': %s\\n", validateEmail("@example.com") ? "true" : "false");
    printf("\\n");
    
    // Test Case 3: Invalid phone number
    printf("Test Case 3: Invalid Phone Number\\n");
    printf("Phone '12345': %s\\n", validatePhoneNumber("12345") ? "true" : "false");
    printf("Phone 'abc-def-ghij': %s\\n", validatePhoneNumber("abc-def-ghij") ? "true" : "false");
    printf("\\n");
    
    // Test Case 4: Invalid password (weak)
    printf("Test Case 4: Invalid Password\\n");
    printf("Password 'weak': %s\\n", validatePassword("weak") ? "true" : "false");
    printf("Password 'NoSpecialChar1': %s\\n", validatePassword("NoSpecialChar1") ? "true" : "false");
    printf("Password 'nouppercas1!': %s\\n", validatePassword("nouppercas1!") ? "true" : "false");
    printf("\\n");
    
    // Test Case 5: Invalid username
    printf("Test Case 5: Invalid Username\\n");
    printf("Username 'ab': %s\\n", validateUsername("ab") ? "true" : "false");
    printf("Username 'user@name': %s\\n", validateUsername("user@name") ? "true" : "false");
    printf("Username 'this_is_way_too_long_username': %s\\n", validateUsername("this_is_way_too_long_username") ? "true" : "false");
    
    return 0;
}
