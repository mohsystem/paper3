#include <stdio.h>
#include <string.h>
#include <stddef.h>

static int is_ascii_letter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static int is_ascii_digit(char c) {
    return (c >= '0' && c <= '9');
}

static int is_local_allowed(char c) {
    // Allowed specials in dot-atom local part (subset)
    switch (c) {
        case '!': case '#': case '$': case '%': case '&':
        case '\'': case '*': case '+': case '/': case '=':
        case '?': case '^': case '_': case '`': case '{':
        case '|': case '}': case '~': case '-':
            return 1;
        default:
            break;
    }
    return is_ascii_letter(c) || is_ascii_digit(c);
}

static int validate_local_part(const char* s, size_t len) {
    if (s == NULL) return 0;
    if (len == 0 || len > 64) return 0;
    if (s[0] == '.' || s[len - 1] == '.') return 0;

    int prevDot = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char uc = (unsigned char)s[i];
        if (uc < 33 || uc > 126) return 0; // printable ASCII excluding space
        if (s[i] == '.') {
            if (prevDot) return 0;
            prevDot = 1;
        } else {
            prevDot = 0;
            if (!is_local_allowed(s[i])) return 0;
        }
    }
    return 1;
}

static int validate_domain(const char* s, size_t len) {
    if (s == NULL) return 0;
    if (len == 0 || len > 253) return 0;
    if (s[0] == '.' || s[len - 1] == '.') return 0;

    int prevDot = 0;
    int hasDot = 0;

    size_t labelLen = 0;
    int labelAllAlpha = 1;
    int labelStarted = 0;
    char lastChar = '\0';

    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        unsigned char uc = (unsigned char)c;
        if (uc < 33 || uc > 126) return 0; // printable ASCII excluding space

        if (c == '.') {
            if (prevDot) return 0;
            if (!labelStarted || labelLen == 0) return 0;
            if (lastChar == '-') return 0;
            if (labelLen > 63) return 0;

            hasDot = 1;
            prevDot = 1;
            labelLen = 0;
            labelAllAlpha = 1;
            labelStarted = 0;
            lastChar = '.';
            continue;
        }

        prevDot = 0;

        if (!(is_ascii_letter(c) || is_ascii_digit(c) || c == '-')) return 0;

        if (!labelStarted) {
            if (c == '-') return 0; // cannot start with hyphen
            labelStarted = 1;
        }

        if (!is_ascii_letter(c)) {
            labelAllAlpha = 0;
        }

        labelLen++;
        lastChar = c;

        if (labelLen > 63) return 0;
    }

    if (!labelStarted || labelLen == 0) return 0;
    if (lastChar == '-') return 0;
    if (labelLen > 63) return 0;
    if (!hasDot) return 0;

    // TLD must be alpha-only and at least 2 chars
    if (!(labelAllAlpha && labelLen >= 2)) return 0;

    return 1;
}

int is_valid_email(const char* email) {
    if (email == NULL) return 0;
    size_t len = strlen(email);
    if (len == 0 || len > 254) return 0;

    size_t atCount = 0;
    size_t atPos = (size_t)(-1);
    for (size_t i = 0; i < len; ++i) {
        if (email[i] == '@') {
            atCount++;
            if (atCount == 1) atPos = i;
        }
    }
    if (atCount != 1) return 0;
    if (atPos == 0 || atPos == len - 1) return 0;

    size_t localLen = atPos;
    size_t domainLen = len - atPos - 1;

    if (!validate_local_part(email, localLen)) return 0;
    if (!validate_domain(email + atPos + 1, domainLen)) return 0;

    return 1;
}

int main(void) {
    const char* tests[5] = {
        "alice.smith+tag@example.co.uk",                 /* valid */
        "bad..dots@domain.com",                          /* invalid */
        "user@-example.com",                             /* invalid */
        "customer/department=shipping@example.com",      /* valid */
        "user@example.c"                                 /* invalid (TLD too short) */
    };

    for (int i = 0; i < 5; ++i) {
        int ok = is_valid_email(tests[i]);
        printf("%s -> %s\n", tests[i], ok ? "valid" : "invalid");
    }
    return 0;
}