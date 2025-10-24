#include <stdio.h>
#include <string.h>
#include <ctype.h>

int is_valid_email(const char* email) {
    if (email == NULL) return 0;
    size_t len = strlen(email);
    if (len == 0 || len > 254) return 0;

    // Find '@' and ensure exactly one
    int at_count = 0;
    int at_index = -1;
    for (size_t i = 0; i < len; i++) {
        if (email[i] == '@') {
            at_count++;
            at_index = (int)i;
        }
    }
    if (at_count != 1 || at_index <= 0 || at_index >= (int)len - 1) return 0;

    int local_len = at_index;
    int domain_len = (int)len - at_index - 1;
    if (local_len <= 0 || local_len > 64) return 0;

    // Local part checks
    if (email[0] == '.' || email[local_len - 1] == '.') return 0;
    char prev = '\0';
    for (int i = 0; i < local_len; i++) {
        char c = email[i];
        int allowed = isalnum((unsigned char)c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
        if (!allowed) return 0;
        if (c == '.' && prev == '.') return 0;
        prev = c;
    }

    // Domain checks
    if (domain_len <= 0) return 0;
    const char* domain = email + at_index + 1;

    int label_count = 0;
    int i = 0;
    int label_start = 0;
    int last_label_start = 0;
    int last_label_end = -1;

    while (i <= domain_len) {
        if (i == domain_len || domain[i] == '.') {
            int label_len = i - label_start;
            if (label_len <= 0 || label_len > 63) return 0;
            // first and last char alnum
            if (!isalnum((unsigned char)domain[label_start]) ||
                !isalnum((unsigned char)domain[i - 1])) return 0;
            // char set
            for (int j = label_start; j < i; j++) {
                char c = domain[j];
                if (!(isalnum((unsigned char)c) || c == '-')) return 0;
            }
            label_count++;
            last_label_start = label_start;
            last_label_end = i - 1;
            label_start = i + 1; // next label starts after '.'
        }
        i++;
    }
    if (label_count < 2) return 0;

    // TLD checks: only letters and length >= 2
    int tld_len = last_label_end - last_label_start + 1;
    if (tld_len < 2) return 0;
    for (int j = last_label_start; j <= last_label_end; j++) {
        if (!isalpha((unsigned char)domain[j])) return 0;
    }

    return 1;
}

int main(void) {
    const char* tests[5] = {
        "user@example.com",
        "first.last+tag@sub.domain.co",
        ".user@domain.com",
        "user@-domain.com",
        "user@domain"
    };
    for (int k = 0; k < 5; k++) {
        printf("%s => %s\n", tests[k], is_valid_email(tests[k]) ? "true" : "false");
    }
    return 0;
}