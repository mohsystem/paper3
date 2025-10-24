#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Helper function to check if a character is a valid for the local-part
bool is_valid_local_char(char c) {
    return isalnum(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
}

// Helper function to check if a character is valid for a domain label
bool is_valid_domain_char(char c) {
    return isalnum(c) || c == '-';
}

bool isValidEmail(const char* email) {
    if (email == NULL) {
        return false;
    }

    size_t len = strlen(email);
    if (len == 0 || len > 254) {
        return false;
    }

    // Find exactly one '@'
    const char* at_symbol = strchr(email, '@');
    if (at_symbol == NULL || strrchr(email, '@') != at_symbol) {
        return false;
    }

    // Validate Local Part
    size_t local_len = at_symbol - email;
    if (local_len < 1 || local_len > 64) {
        return false;
    }
    const char* local_part = email;
    if (local_part[0] == '.' || local_part[local_len - 1] == '.') {
        return false;
    }
    for (size_t i = 0; i < local_len; i++) {
        if (!is_valid_local_char(local_part[i])) {
            return false;
        }
        if (local_part[i] == '.' && i + 1 < local_len && local_part[i+1] == '.') {
            return false;
        }
    }

    // Validate Domain Part
    size_t domain_len = len - local_len - 1;
    if (domain_len < 1 || domain_len > 255) {
        return false;
    }
    const char* domain_part = at_symbol + 1;
    const char* last_dot = strrchr(domain_part, '.');
    
    if (last_dot == NULL || last_dot == domain_part) {
        return false; // Domain must have a dot, not at the beginning
    }

    // Validate TLD (part after the last dot)
    size_t tld_len = strlen(last_dot + 1);
    if (tld_len < 2) {
        return false;
    }
    for (size_t i = 0; i < tld_len; i++) {
        if (!isalpha((unsigned char)last_dot[i + 1])) {
            return false;
        }
    }

    // Validate domain labels (parts between dots)
    const char* current = domain_part;
    while(current < last_dot) {
        const char* next_dot = strchr(current, '.');
        if (next_dot == NULL) { // Should be caught by last_dot check
             break;
        }
        size_t label_len = next_dot - current;
        if (label_len < 1) { // e.g., "domain..com"
            return false;
        }
        if (current[0] == '-' || current[label_len - 1] == '-') {
            return false;
        }
        for (size_t i = 0; i < label_len; i++) {
            if (!is_valid_domain_char(current[i])) {
                return false;
            }
        }
        current = next_dot + 1;
    }

    return true;
}

int main() {
    const char* testEmails[] = {
        "test@example.com",           // Valid
        "test.name@example.co.uk",  // Valid
        "test..name@example.com",   // Invalid (consecutive dots)
        "test@example",             // Invalid (no TLD)
        "plainaddress"              // Invalid (no '@' or domain)
    };
    int num_tests = sizeof(testEmails) / sizeof(testEmails[0]);

    printf("C Email Validation:\n");
    for (int i = 0; i < num_tests; i++) {
        printf("'%s' is valid: %s\n", testEmails[i], isValidEmail(testEmails[i]) ? "true" : "false");
    }

    return 0;
}