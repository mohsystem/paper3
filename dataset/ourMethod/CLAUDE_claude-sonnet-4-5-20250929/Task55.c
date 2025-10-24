
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

// Maximum email length according to RFC 5321
#define MAX_EMAIL_LENGTH 320
#define MIN_EMAIL_LENGTH 3
#define MAX_LOCAL_LENGTH 64
#define MAX_DOMAIN_LENGTH 255

// Email validation function - validates format and structure
// Returns true (1) if valid, false (0) otherwise
// Security: All inputs validated, no buffer overflows, bounds checked
bool validate_email(const char* email) {
    // Security: Validate input pointer
    if (email == NULL) {
        return false;
    }
    
    // Security: Check length to prevent buffer overflow and DoS
    size_t len = strnlen(email, MAX_EMAIL_LENGTH + 1);
    if (len < MIN_EMAIL_LENGTH || len > MAX_EMAIL_LENGTH) {
        return false;
    }
    
    // Security: Check for null bytes in middle of string (injection prevention)
    for (size_t i = 0; i < len; i++) {
        if (email[i] == '\\0') {
            return false;
        }
    }
    
    // Find @ symbol position and count
    size_t at_pos = 0;
    int at_count = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (email[i] == '@') {
            at_count++;
            at_pos = i;
        }
    }
    
    // Must have exactly one @ symbol
    if (at_count != 1) {
        return false;
    }
    
    // Validate @ position (not at start or end)
    if (at_pos == 0 || at_pos >= len - 1) {
        return false;
    }
    
    // Validate local part length
    if (at_pos > MAX_LOCAL_LENGTH) {
        return false;
    }
    
    // Validate domain part length
    size_t domain_len = len - at_pos - 1;
    if (domain_len > MAX_DOMAIN_LENGTH || domain_len < 4) { // min: a.bc
        return false;
    }
    
    // Validate local part (before @)
    if (email[0] == '.' || email[at_pos - 1] == '.') {
        return false; // No leading/trailing dots
    }
    
    for (size_t i = 0; i < at_pos; i++) {
        char c = email[i];
        
        // Check for consecutive dots
        if (c == '.' && i + 1 < at_pos && email[i + 1] == '.') {
            return false;
        }
        
        // Valid characters: alphanumeric, dot, hyphen, underscore
        if (!isalnum((unsigned char)c) && c != '.' && c != '-' && c != '_') {
            return false;
        }
    }
    
    // Validate domain part (after @)
    size_t domain_start = at_pos + 1;
    size_t last_dot = 0;
    bool has_dot = false;
    
    if (email[domain_start] == '.' || email[len - 1] == '.') {
        return false; // No leading/trailing dots in domain
    }
    
    for (size_t i = domain_start; i < len; i++) {
        char c = email[i];
        
        if (c == '.') {
            has_dot = true;
            last_dot = i;
            
            // Check for consecutive dots
            if (i + 1 < len && email[i + 1] == '.') {
                return false;
            }
        } else if (!isalnum((unsigned char)c) && c != '-') {
            return false; // Invalid character in domain
        }
    }
    
    // Domain must have at least one dot
    if (!has_dot || last_dot == domain_start) {
        return false;
    }
    
    // Validate TLD length (minimum 2 characters)
    size_t tld_len = len - last_dot - 1;
    if (tld_len < 2 || tld_len > 63) {
        return false;
    }
    
    // Validate TLD contains only letters
    for (size_t i = last_dot + 1; i < len; i++) {
        if (!isalpha((unsigned char)email[i])) {
            return false;
        }
    }
    
    return true;
}

int main(void) {
    // Test cases - all strings are null-terminated and within bounds
    const char* test_cases[5] = {
        "user@example.com",
        "test.email@domain.co.uk",
        "invalid.email@",
        "@nodomain.com",
        "no.at.sign.com"
    };
    
    printf("Email Validation Results:\\n");
    printf("========================\\n");
    
    for (int i = 0; i < 5; i++) {
        // Security: Validate array bounds
        if (i < 0 || i >= 5) {
            fprintf(stderr, "Array index out of bounds\\n");
            continue;
        }
        
        const char* email = test_cases[i];
        
        // Security: Validate pointer before use
        if (email == NULL) {
            fprintf(stderr, "Null email pointer at index %d\\n", i);
            continue;
        }
        
        bool is_valid = validate_email(email);
        
        // Security: Use snprintf with bounds checking and const format string
        char result[512];
        int written = snprintf(result, sizeof(result), 
                              "Email: \\"%s\\" - %s\\n", 
                              email, 
                              is_valid ? "VALID" : "INVALID");
        
        // Security: Check snprintf return value for truncation
        if (written < 0 || (size_t)written >= sizeof(result)) {
            fprintf(stderr, "Output truncated or error\\n");
            continue;
        }
        
        printf("%s", result);
    }
    
    return 0;
}
