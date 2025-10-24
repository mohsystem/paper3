#include <stdio.h>
#include <string.h>

static int is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
static int is_digit(char c) {
    return (c >= '0' && c <= '9');
}
static int is_local_allowed(char c) {
    return is_alpha(c) || is_digit(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
}
static int is_domain_label_char(char c) {
    return is_alpha(c) || is_digit(c) || c == '-';
}

int is_valid_email(const char* email) {
    if (email == NULL) return 0;
    size_t len = strlen(email);
    if (len == 0 || len > 254) return 0;

    const char* at = strchr(email, '@');
    if (at == NULL || at == email) return 0;
    if (strchr(at + 1, '@') != NULL) return 0;

    size_t local_len = (size_t)(at - email);
    const char* local = email;
    const char* domain = at + 1;
    size_t domain_len = len - local_len - 1;

    if (local_len == 0 || domain_len == 0) return 0;
    if (local_len > 64) return 0;

    if (local[0] == '.' || local[local_len - 1] == '.') return 0;
    for (size_t i = 0; i < local_len; i++) {
        char c = local[i];
        if (!is_local_allowed(c)) return 0;
        if (i + 1 < local_len && c == '.' && local[i + 1] == '.') return 0;
    }

    if (domain[0] == '.' || domain[domain_len - 1] == '.') return 0;
    int has_dot = 0;
    for (size_t i = 0; i < domain_len; i++) {
        char c = domain[i];
        if (!(is_alpha(c) || is_digit(c) || c == '-' || c == '.')) return 0;
        if (i + 1 < domain_len && c == '.' && domain[i + 1] == '.') return 0;
        if (c == '.') has_dot = 1;
    }
    if (!has_dot) return 0;

    // Label checks
    size_t i = 0;
    size_t label_len = 0;
    char prev = 0;
    while (i < domain_len) {
        char c = domain[i];
        if (c == '.') {
            if (label_len == 0) return 0;
            if (prev == '-') return 0;
            label_len = 0;
        } else {
            if (!is_domain_label_char(c)) return 0;
            if (label_len == 0 && c == '-') return 0;
            label_len++;
            if (label_len > 63) return 0;
        }
        prev = c;
        i++;
    }
    if (label_len == 0) return 0;
    if (prev == '-') return 0;

    // TLD checks: last label all letters and length >= 2
    const char* last_dot = strrchr(domain, '.');
    const char* tld = last_dot ? last_dot + 1 : domain;
    size_t tld_len = strlen(tld);
    if (tld_len < 2) return 0;
    for (i = 0; i < tld_len; i++) {
        if (!is_alpha(tld[i])) return 0;
    }

    return 1;
}

int main(void) {
    const char* tests[5] = {
        "user.name+tag@example.com",
        "user@sub.example.co",
        "user..name@example.com",
        "user@-example.com",
        "user@example"
    };

    for (int i = 0; i < 5; i++) {
        printf("%s -> %s\n", tests[i], is_valid_email(tests[i]) ? "true" : "false");
    }
    return 0;
}