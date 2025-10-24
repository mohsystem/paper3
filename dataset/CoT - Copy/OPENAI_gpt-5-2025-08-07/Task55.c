/* Chain-of-Through secure implementation for email validation in C */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static bool is_alnum_ascii(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9');
}

static bool is_alpha_ascii(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

static bool validate_local(const char* s, size_t start, size_t len) {
    if (len == 0 || len > 64) return false;
    if (s[start] == '.' || s[start + len - 1] == '.') return false;
    bool prev_dot = false;
    for (size_t i = 0; i < len; ++i) {
        char c = s[start + i];
        if (!(is_alnum_ascii(c) || c == '.' || c == '_' || c == '+' || c == '-')) return false;
        if (c == '.') {
            if (prev_dot) return false;
            prev_dot = true;
        } else {
            prev_dot = false;
        }
    }
    return true;
}

static bool validate_domain(const char* s, size_t start, size_t len) {
    if (len == 0 || len > 253) return false;

    size_t i = start;
    size_t end = start + len;
    size_t label_count = 0;

    size_t last_label_start = 0;
    size_t last_label_len = 0;

    while (i < end) {
        size_t label_start = i;
        size_t label_len = 0;

        while (i < end && s[i] != '.') {
            char c = s[i];
            if (!(is_alnum_ascii(c) || c == '-')) return false;
            ++i;
            ++label_len;
            if (label_len > 63) return false;
        }

        if (label_len == 0) return false; // empty label or consecutive dots

        if (!is_alnum_ascii(s[label_start]) || !is_alnum_ascii(s[label_start + label_len - 1])) return false;

        label_count++;
        last_label_start = label_start;
        last_label_len = label_len;

        if (i < end && s[i] == '.') {
            i++; // skip dot
            if (i == end) return false; // trailing dot -> empty label
        }
    }

    if (label_count < 2) return false;

    // TLD checks
    if (last_label_len < 2 || last_label_len > 63) return false;
    for (size_t k = 0; k < last_label_len; ++k) {
        char c = s[last_label_start + k];
        if (!is_alpha_ascii(c)) return false;
    }

    return true;
}

bool validate_email(const char* email) {
    if (email == NULL) return false;

    size_t len = strlen(email);
    if (len == 0 || len > 254) return false;

    for (size_t i = 0; i < len; ++i) {
        unsigned char uc = (unsigned char)email[i];
        if (uc <= 32 || uc >= 127) return false; // reject control, space, non-ASCII
    }

    // find '@'
    size_t at_first = (size_t)-1, at_last = (size_t)-1;
    for (size_t i = 0; i < len; ++i) {
        if (email[i] == '@') {
            if (at_first == (size_t)-1) at_first = i;
            at_last = i;
        }
    }
    if (at_first == (size_t)-1 || at_first != at_last) return false;
    if (at_first == 0 || at_first == len - 1) return false;

    size_t local_start = 0;
    size_t local_len = at_first - local_start;
    size_t domain_start = at_first + 1;
    size_t domain_len = len - domain_start;

    if (!validate_local(email, local_start, local_len)) return false;
    if (!validate_domain(email, domain_start, domain_len)) return false;

    return true;
}

static void run_test(const char* s) {
    printf("%s -> %s\n", s, validate_email(s) ? "true" : "false");
}

int main(void) {
    // 5 test cases
    run_test("user.name+tag-123@example-domain.com"); // valid
    run_test("a_b@example.co");                       // valid
    run_test("user..dot@example.com");                // invalid
    run_test("user@example");                         // invalid
    run_test(".startingdot@example.com");             // invalid
    return 0;
}