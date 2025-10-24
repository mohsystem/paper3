#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_allowed(int c) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 'a' && uc <= 'z') return 1;
    if (uc >= 'A' && uc <= 'Z') return 1;
    if (uc >= '0' && uc <= '9') return 1;
    if (uc == ' ' || uc == '\t') return 1;
    switch (uc) {
        case '_':
        case '-':
        case '.':
        case ',':
        case ':':
        case '@':
            return 1;
        default:
            return 0;
    }
}

static char* safe_strdup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* d = (char*)malloc(n + 1);
    if (!d) return NULL;
    memcpy(d, s, n + 1);
    return d;
}

char* process_input(const char* input) {
    const char* INVALID = "Invalid input";
    if (input == NULL) {
        return safe_strdup(INVALID);
    }
    size_t in_len = strlen(input);
    if (in_len > 256) {
        return safe_strdup(INVALID);
    }
    for (size_t i = 0; i < in_len; i++) {
        if (!is_allowed((unsigned char)input[i])) {
            return safe_strdup(INVALID);
        }
    }
    char* sanitized = (char*)malloc(in_len + 1);
    if (!sanitized) {
        return safe_strdup(INVALID);
    }
    size_t j = 0;
    int lastWasSpace = 1; // trim leading and collapse
    for (size_t i = 0; i < in_len; i++) {
        char ch = input[i];
        if (ch == ' ' || ch == '\t') {
            if (!lastWasSpace) {
                sanitized[j++] = ' ';
                lastWasSpace = 1;
            }
        } else {
            sanitized[j++] = ch;
            lastWasSpace = 0;
        }
    }
    if (j > 0 && sanitized[j - 1] == ' ') {
        j--;
    }
    sanitized[j] = '\0';
    if (j == 0) {
        free(sanitized);
        return safe_strdup(INVALID);
    }
    size_t digit_count = 0;
    for (size_t i = 0; i < j; i++) {
        char ch = sanitized[i];
        if (ch >= '0' && ch <= '9') digit_count++;
    }
    char* reversed = (char*)malloc(j + 1);
    if (!reversed) {
        free(sanitized);
        return safe_strdup(INVALID);
    }
    for (size_t i = 0; i < j; i++) {
        reversed[i] = sanitized[j - 1 - i];
    }
    reversed[j] = '\0';

    size_t out_cap = j + j + 100;
    char* out = (char*)malloc(out_cap);
    if (!out) {
        free(sanitized);
        free(reversed);
        return safe_strdup(INVALID);
    }
    int written = snprintf(out, out_cap, "sanitized=%s; length=%zu; digits=%zu; reversed=%s",
                           sanitized, j, digit_count, reversed);
    if (written < 0 || (size_t)written >= out_cap) {
        free(out);
        free(sanitized);
        free(reversed);
        return safe_strdup(INVALID);
    }
    free(sanitized);
    free(reversed);
    return out;
}

int main(void) {
    const char* tests[5] = {
        "Hello World",
        "  Multiple    spaces   123  ",
        "Invalid!Chars#",
        "A_Very-Long.String,With:Allowed@Chars",
        ""
    };
    for (int i = 0; i < 5; i++) {
        char* res = process_input(tests[i]);
        if (res) {
            printf("Input: \"%s\" -> %s\n", tests[i], res);
            free(res);
        } else {
            printf("Input: \"%s\" -> %s\n", tests[i], "Invalid input");
        }
    }
    return 0;
}