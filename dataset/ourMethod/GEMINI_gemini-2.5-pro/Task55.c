#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

#define TRUE 1
#define FALSE 0

// Helper function to check if a character is a valid special character in the local part
int isLocalPartSpecialChar(char c) {
    return c == '.' || c == '_' || c == '+' || c == '-';
}

// Function that validates an email address
int isValidEmail(const char* email) {
    if (email == NULL) {
        return FALSE;
    }

    size_t len = strlen(email);
    // 1. Total length check
    if (len > 256 || len < 3) {
        return FALSE;
    }

    // 2. Find the '@' symbol. Must be exactly one.
    const char* at_ptr = strchr(email, '@');
    if (at_ptr == NULL || strchr(at_ptr + 1, '@') != NULL) {
        return FALSE; // No '@' or more than one '@'
    }

    // 3. Determine local and domain part positions
    size_t at_pos = at_ptr - email;
    const char* domain_part = at_ptr + 1;

    // 4. Validate Local Part
    size_t local_len = at_pos;
    if (local_len == 0 || local_len > 64) {
        return FALSE;
    }
    if (isLocalPartSpecialChar(email[0]) || isLocalPartSpecialChar(email[at_pos - 1])) {
        return FALSE; // Cannot start or end with a special character
    }
    for (size_t i = 0; i < local_len; ++i) {
        char c = email[i];
        if (!isalnum((unsigned char)c) && !isLocalPartSpecialChar(c)) {
            return FALSE; // Invalid character
        }
        if (isLocalPartSpecialChar(c) && (i + 1 < local_len) && isLocalPartSpecialChar(email[i + 1])) {
            return FALSE; // Consecutive special characters
        }
    }

    // 5. Validate Domain Part
    size_t domain_len = len - at_pos - 1;
    if (domain_len == 0 || domain_len > 255) {
        return FALSE;
    }
    // Domain part cannot start or end with a dot or hyphen
    if (domain_part[0] == '.' || domain_part[0] == '-' || domain_part[domain_len - 1] == '.' || domain_part[domain_len - 1] == '-') {
        return FALSE;
    }

    int dot_found = FALSE;
    size_t label_start = 0;
    for (size_t i = 0; i < domain_len; ++i) {
        char c = domain_part[i];
        if (c == '.') {
            dot_found = TRUE;
            if (i == label_start) return FALSE; // Empty label (e.g., domain..com)
            
            // Check the label from label_start to i-1
            if (domain_part[label_start] == '-' || domain_part[i - 1] == '-') {
                return FALSE; // Label starts or ends with a hyphen
            }
            if ((i - label_start) > 63) return FALSE; // Label too long

            label_start = i + 1;
        } else if (!isalnum((unsigned char)c) && c != '-') {
            return FALSE; // Invalid character in domain
        }
    }

    if (!dot_found) return FALSE; // Must have at least one dot

    // Validate the TLD (the last label)
    if (label_start >= domain_len) return FALSE; // No TLD found (e.g., domain.)

    size_t tld_len = domain_len - label_start;
    if (tld_len < 2 || tld_len > 63) return FALSE; // TLD length constraints

    for (size_t i = label_start; i < domain_len; ++i) {
        if (!isalpha((unsigned char)domain_part[i])) {
            return FALSE; // TLD must be all letters
        }
    }

    return TRUE;
}

int main() {
    const char* emailsToTest[] = {
        "valid.email@example.com",
        "invalid-email@",
        "@invalid.com",
        "another..invalid@email.com",
        "valid-user@sub.domain.co"
    };

    printf("--- 5 Test Cases ---\n");
    for (int i = 0; i < 5; ++i) {
        printf("Email: \"%s\" -> %s\n", emailsToTest[i], 
               (isValidEmail(emailsToTest[i]) ? "Valid" : "Invalid"));
    }

    return 0;
}