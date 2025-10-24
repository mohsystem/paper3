
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_EMAIL_LENGTH 320
#define MAX_LOCAL_LENGTH 64
#define MAX_DOMAIN_LENGTH 255

// Function to trim whitespace from string
void trim(char* str) {
    int start = 0;
    int end = strlen(str) - 1;
    
    while (isspace((unsigned char)str[start])) start++;
    while (end >= start && isspace((unsigned char)str[end])) end--;
    
    int i;
    for (i = 0; i <= end - start; i++) {
        str[i] = str[start + i];
    }
    str[i] = '\\0';
}

// Function to count occurrences of a character
int countChar(const char* str, char c) {
    int count = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == c) count++;
    }
    return count;
}

// Function to check if character is valid for local part
bool isValidLocalChar(char c) {
    return isalnum((unsigned char)c) || 
           c == '.' || c == '!' || c == '#' || c == '$' || 
           c == '%' || c == '&' || c == '\\'' || c == '*' || 
           c == '+' || c == '/' || c == '=' || c == '?' || 
           c == '^' || c == '_' || c == '`' || c == '{' || 
           c == '|' || c == '}' || c == '~' || c == '-';
}

// Function to check if character is valid for domain part
bool isValidDomainChar(char c) {
    return isalnum((unsigned char)c) || c == '.' || c == '-';
}

bool isValidEmail(const char* input) {
    // Input validation
    if (input == NULL || strlen(input) == 0) {
        return false;
    }
    
    char email[MAX_EMAIL_LENGTH + 1];
    strncpy(email, input, MAX_EMAIL_LENGTH);
    email[MAX_EMAIL_LENGTH] = '\\0';
    trim(email);
    
    int len = strlen(email);
    
    // Check maximum length to prevent DoS
    if (len > MAX_EMAIL_LENGTH || len == 0) {
        return false;
    }
    
    // Check for exactly one @ symbol
    int atCount = countChar(email, '@');
    if (atCount != 1) {
        return false;
    }
    
    // Find @ position
    int atPosition = -1;
    for (int i = 0; i < len; i++) {
        if (email[i] == '@') {
            atPosition = i;
            break;
        }
    }
    
    if (atPosition == 0 || atPosition == len - 1) {
        return false;
    }
    
    // Extract local and domain parts
    char localPart[MAX_LOCAL_LENGTH + 1];
    char domainPart[MAX_DOMAIN_LENGTH + 1];
    
    strncpy(localPart, email, atPosition);
    localPart[atPosition] = '\\0';
    strcpy(domainPart, email + atPosition + 1);
    
    int localLen = strlen(localPart);
    int domainLen = strlen(domainPart);
    
    // Validate lengths
    if (localLen > MAX_LOCAL_LENGTH || domainLen > MAX_DOMAIN_LENGTH) {
        return false;
    }
    
    // Validate local part
    for (int i = 0; i < localLen; i++) {
        if (!isValidLocalChar(localPart[i])) {
            return false;
        }
    }
    
    // Check for consecutive dots or leading/trailing dots in local part
    if (localPart[0] == '.' || localPart[localLen - 1] == '.' || 
        strstr(localPart, "..") != NULL) {
        return false;
    }
    
    // Validate domain part
    if (domainPart[0] == '.' || domainPart[0] == '-' || 
        domainPart[domainLen - 1] == '.' || domainPart[domainLen - 1] == '-') {
        return false;
    }
    
    for (int i = 0; i < domainLen; i++) {
        if (!isValidDomainChar(domainPart[i])) {
            return false;
        }
    }
    
    // Check for at least one dot in domain
    if (strchr(domainPart, '.') == NULL) {
        return false;
    }
    
    // Find and validate TLD
    char* lastDot = strrchr(domainPart, '.');
    if (lastDot == NULL) {
        return false;
    }
    
    int tldLen = strlen(lastDot + 1);
    if (tldLen < 2 || tldLen > 63) {
        return false;
    }
    
    return true;
}

int main() {
    // Test cases
    const char* testEmails[] = {
        "user@example.com",
        "test.email@domain.co.uk",
        "invalid.email@",
        "@invalid.com",
        "no-at-sign.com"
    };
    
    int numTests = sizeof(testEmails) / sizeof(testEmails[0]);
    
    printf("Email Validation Test Results:\\n");
    printf("==================================================\\n");
    
    for (int i = 0; i < numTests; i++) {
        printf("%-50s : %s\\n", testEmails[i], 
               isValidEmail(testEmails[i]) ? "VALID" : "INVALID");
    }
    
    return 0;
}
