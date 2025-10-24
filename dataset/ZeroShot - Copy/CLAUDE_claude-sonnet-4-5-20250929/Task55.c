
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool isValidEmail(const char* email) {
    if (email == NULL || strlen(email) == 0) {
        return false;
    }
    
    int len = strlen(email);
    int atPos = -1;
    int dotPos = -1;
    
    // Find @ position
    for (int i = 0; i < len; i++) {
        if (email[i] == '@') {
            if (atPos != -1) {
                return false; // Multiple @ symbols
            }
            atPos = i;
        }
    }
    
    // Check if @ exists and not at start or end
    if (atPos <= 0 || atPos >= len - 1) {
        return false;
    }
    
    // Validate local part (before @)
    for (int i = 0; i < atPos; i++) {
        char c = email[i];
        if (!isalnum(c) && c != '.' && c != '_' && c != '+' && c != '&' && c != '*' && c != '-') {
            return false;
        }
    }
    
    // Find last dot after @
    for (int i = atPos + 1; i < len; i++) {
        if (email[i] == '.') {
            dotPos = i;
        }
    }
    
    // Check if dot exists after @ and not immediately after @
    if (dotPos <= atPos + 1 || dotPos >= len - 1) {
        return false;
    }
    
    // Validate domain part (after @)
    for (int i = atPos + 1; i < len; i++) {
        char c = email[i];
        if (!isalnum(c) && c != '.' && c != '-') {
            return false;
        }
    }
    
    // Check TLD length (after last dot)
    int tldLen = len - dotPos - 1;
    if (tldLen < 2 || tldLen > 7) {
        return false;
    }
    
    return true;
}

int main() {
    // Test cases
    const char* testEmails[] = {
        "user@example.com",
        "john.doe@company.co.uk",
        "invalid.email@",
        "@invalid.com",
        "valid_email123@test-domain.org"
    };
    
    int numTests = sizeof(testEmails) / sizeof(testEmails[0]);
    
    printf("Email Validation Results:\\n");
    for (int i = 0; i < numTests; i++) {
        printf("%s -> %s\\n", testEmails[i], isValidEmail(testEmails[i]) ? "Valid" : "Invalid");
    }
    
    return 0;
}
