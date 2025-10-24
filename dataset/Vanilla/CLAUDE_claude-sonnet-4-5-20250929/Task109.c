
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

typedef struct {
    bool isValid;
    char message[200];
} ValidationResult;

ValidationResult validateEmail(const char* email) {
    ValidationResult result;
    regex_t regex;
    int reti;
    
    reti = regcomp(&regex, "^[a-zA-Z0-9_+&*-]+(\\\\.[a-zA-Z0-9_+&*-]+)*@([a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$", REG_EXTENDED);
    
    reti = regexec(&regex, email, 0, NULL, 0);
    
    if (!reti) {
        result.isValid = true;
        strcpy(result.message, "Valid email address");
    } else {
        result.isValid = false;
        strcpy(result.message, "Invalid email address");
    }
    
    regfree(&regex);
    return result;
}

ValidationResult validatePhone(const char* phone) {
    ValidationResult result;
    regex_t regex;
    int reti;
    
    reti = regcomp(&regex, "^(\\\\+?[1-9][0-9]{1,14}|\\\\(?[0-9]{3}\\\\)?[-. ]?[0-9]{3}[-. ]?[0-9]{4})$", REG_EXTENDED);
    
    reti = regexec(&regex, phone, 0, NULL, 0);
    
    if (!reti) {
        result.isValid = true;
        strcpy(result.message, "Valid phone number");
    } else {
        result.isValid = false;
        strcpy(result.message, "Invalid phone number");
    }
    
    regfree(&regex);
    return result;
}

ValidationResult validatePassword(const char* password) {
    ValidationResult result;
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    int len = strlen(password);
    
    if (len < 8) {
        result.isValid = false;
        strcpy(result.message, "Invalid password (must be 8+ chars with uppercase, lowercase, digit, and special character)");
        return result;
    }
    
    for (int i = 0; i < len; i++) {
        if (password[i] >= 'a' && password[i] <= 'z') hasLower = true;
        else if (password[i] >= 'A' && password[i] <= 'Z') hasUpper = true;
        else if (password[i] >= '0' && password[i] <= '9') hasDigit = true;
        else if (strchr("@$!%*?&", password[i])) hasSpecial = true;
    }
    
    if (hasLower && hasUpper && hasDigit && hasSpecial) {
        result.isValid = true;
        strcpy(result.message, "Valid password");
    } else {
        result.isValid = false;
        strcpy(result.message, "Invalid password (must be 8+ chars with uppercase, lowercase, digit, and special character)");
    }
    
    return result;
}

ValidationResult validateUsername(const char* username) {
    ValidationResult result;
    regex_t regex;
    int reti;
    
    reti = regcomp(&regex, "^[a-zA-Z0-9_]{3,20}$", REG_EXTENDED);
    
    reti = regexec(&regex, username, 0, NULL, 0);
    
    if (!reti) {
        result.isValid = true;
        strcpy(result.message, "Valid username");
    } else {
        result.isValid = false;
        strcpy(result.message, "Invalid username (must be 3-20 alphanumeric characters or underscore)");
    }
    
    regfree(&regex);
    return result;
}

ValidationResult validateURL(const char* url) {
    ValidationResult result;
    regex_t regex;
    int reti;
    
    reti = regcomp(&regex, "^(https?|ftp)://[^ /$.?#].[^ ]*$", REG_EXTENDED | REG_ICASE);
    
    reti = regexec(&regex, url, 0, NULL, 0);
    
    if (!reti) {
        result.isValid = true;
        strcpy(result.message, "Valid URL");
    } else {
        result.isValid = false;
        strcpy(result.message, "Invalid URL");
    }
    
    regfree(&regex);
    return result;
}

int main() {
    printf("===== Test Case 1: Email Validation =====\\n");
    ValidationResult result1 = validateEmail("user@example.com");
    printf("Input: user@example.com\\n");
    printf("Result: %s - %s\\n\\n", result1.isValid ? "true" : "false", result1.message);
    
    printf("===== Test Case 2: Phone Validation =====\\n");
    ValidationResult result2 = validatePhone("123-456-7890");
    printf("Input: 123-456-7890\\n");
    printf("Result: %s - %s\\n\\n", result2.isValid ? "true" : "false", result2.message);
    
    printf("===== Test Case 3: Password Validation =====\\n");
    ValidationResult result3 = validatePassword("SecurePass123!");
    printf("Input: SecurePass123!\\n");
    printf("Result: %s - %s\\n\\n", result3.isValid ? "true" : "false", result3.message);
    
    printf("===== Test Case 4: Username Validation =====\\n");
    ValidationResult result4 = validateUsername("john_doe123");
    printf("Input: john_doe123\\n");
    printf("Result: %s - %s\\n\\n", result4.isValid ? "true" : "false", result4.message);
    
    printf("===== Test Case 5: URL Validation =====\\n");
    ValidationResult result5 = validateURL("https://www.example.com");
    printf("Input: https://www.example.com\\n");
    printf("Result: %s - %s\\n", result5.isValid ? "true" : "false", result5.message);
    
    return 0;
}
