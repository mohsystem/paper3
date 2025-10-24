
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

// Validate email address
bool validateEmail(const char* email) {
    if (email == NULL || strlen(email) == 0) {
        return false;
    }
    regex_t regex;
    int ret;
    ret = regcomp(&regex, "^[a-zA-Z0-9_+&*-]+(\\\\.)[a-zA-Z0-9_+&*-]+)*@([a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$", REG_EXTENDED);
    if (ret) {
        return false;
    }
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

// Validate phone number (US format)
bool validatePhone(const char* phone) {
    if (phone == NULL || strlen(phone) == 0) {
        return false;
    }
    regex_t regex;
    int ret;
    ret = regcomp(&regex, "^(\\\\+1)?[-. ]?\\\\(?[0-9]{3}\\\\)?[-. ]?[0-9]{3}[-. ]?[0-9]{4}$", REG_EXTENDED);
    if (ret) {
        return false;
    }
    ret = regexec(&regex, phone, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

// Validate password - simplified version for C
bool validatePassword(const char* password) {
    if (password == NULL || strlen(password) < 8) {
        return false;
    }
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (int i = 0; password[i] != '\\0'; i++) {
        if (password[i] >= 'A' && password[i] <= 'Z') hasUpper = true;
        else if (password[i] >= 'a' && password[i] <= 'z') hasLower = true;
        else if (password[i] >= '0' && password[i] <= '9') hasDigit = true;
        else if (strchr("@$!%*?&", password[i]) != NULL) hasSpecial = true;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

// Validate URL
bool validateURL(const char* url) {
    if (url == NULL || strlen(url) == 0) {
        return false;
    }
    regex_t regex;
    int ret;
    ret = regcomp(&regex, "^(https?|ftp)://[^ /$.?#].[^ ]*$", REG_EXTENDED | REG_ICASE);
    if (ret) {
        return false;
    }
    ret = regexec(&regex, url, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

// Validate username (alphanumeric and underscore, 3-16 chars)
bool validateUsername(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return false;
    }
    regex_t regex;
    int ret;
    ret = regcomp(&regex, "^[a-zA-Z0-9_]{3,16}$", REG_EXTENDED);
    if (ret) {
        return false;
    }
    ret = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

int main() {
    printf("=== Test Case 1: Email Validation ===\\n");
    const char* email1 = "user@example.com";
    const char* email2 = "invalid.email@";
    printf("%s is valid: %s\\n", email1, validateEmail(email1) ? "true" : "false");
    printf("%s is valid: %s\\n", email2, validateEmail(email2) ? "true" : "false");
    
    printf("\\n=== Test Case 2: Phone Validation ===\\n");
    const char* phone1 = "(123) 456-7890";
    const char* phone2 = "123-456-7890";
    printf("%s is valid: %s\\n", phone1, validatePhone(phone1) ? "true" : "false");
    printf("%s is valid: %s\\n", phone2, validatePhone(phone2) ? "true" : "false");
    
    printf("\\n=== Test Case 3: Password Validation ===\\n");
    const char* pwd1 = "SecurePass123!";
    const char* pwd2 = "weak";
    printf("%s is valid: %s\\n", pwd1, validatePassword(pwd1) ? "true" : "false");
    printf("%s is valid: %s\\n", pwd2, validatePassword(pwd2) ? "true" : "false");
    
    printf("\\n=== Test Case 4: URL Validation ===\\n");
    const char* url1 = "https://www.example.com";
    const char* url2 = "not a url";
    printf("%s is valid: %s\\n", url1, validateURL(url1) ? "true" : "false");
    printf("%s is valid: %s\\n", url2, validateURL(url2) ? "true" : "false");
    
    printf("\\n=== Test Case 5: Username Validation ===\\n");
    const char* user1 = "john_doe123";
    const char* user2 = "ab";
    printf("%s is valid: %s\\n", user1, validateUsername(user1) ? "true" : "false");
    printf("%s is valid: %s\\n", user2, validateUsername(user2) ? "true" : "false");
    
    return 0;
}
