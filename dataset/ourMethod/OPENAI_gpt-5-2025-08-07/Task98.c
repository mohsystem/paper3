#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_LEN 64

typedef struct {
    int success; /* 1 on success, 0 on failure */
    int value;
    char error[64];
} ParseResult;

ParseResult parse_int_strict(const char *input) {
    ParseResult res;
    res.success = 0;
    res.value = 0;
    res.error[0] = '\0';

    if (input == NULL) {
        snprintf(res.error, sizeof(res.error), "%s", "Input is null");
        return res;
    }

    /* Trim leading and trailing whitespace */
    const char *start = input;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }
    const char *endp = start;
    const char *last_nonspace = NULL;
    while (*endp != '\0') {
        if (!isspace((unsigned char)*endp)) {
            last_nonspace = endp;
        }
        endp++;
    }
    size_t trimmed_len = (last_nonspace == NULL) ? 0u : (size_t)(last_nonspace - start + 1);

    if (trimmed_len == 0u) {
        snprintf(res.error, sizeof(res.error), "%s", "Empty input");
        return res;
    }
    if (trimmed_len > MAX_LEN) {
        snprintf(res.error, sizeof(res.error), "%s", "Input too long");
        return res;
    }

    char buf[MAX_LEN + 1];
    for (size_t i = 0; i < trimmed_len; i++) {
        buf[i] = start[i];
    }
    buf[trimmed_len] = '\0';

    size_t idx = 0u;
    int negative = 0;
    if (buf[idx] == '+' || buf[idx] == '-') {
        negative = (buf[idx] == '-');
        idx++;
        if (idx >= trimmed_len) {
            snprintf(res.error, sizeof(res.error), "%s", "No digits after sign");
            return res;
        }
    }

    long long acc = 0;
    for (size_t i = idx; i < trimmed_len; i++) {
        unsigned char c = (unsigned char)buf[i];
        if (c < '0' || c > '9') {
            snprintf(res.error, sizeof(res.error), "%s", "Invalid character encountered");
            return res;
        }
        int d = (int)(c - '0');
        acc = acc * 10 + d;
        long long limit = negative ? (-(long long)INT_MIN) : (long long)INT_MAX;
        if (acc > limit) {
            snprintf(res.error, sizeof(res.error), "%s", "Integer out of range");
            return res;
        }
    }

    long long signedVal = negative ? -acc : acc;
    if (signedVal < (long long)INT_MIN || signedVal > (long long)INT_MAX) {
        snprintf(res.error, sizeof(res.error), "%s", "Integer out of range");
        return res;
    }

    res.success = 1;
    res.value = (int)signedVal;
    res.error[0] = '\0';
    return res;
}

int main(void) {
    for (int i = 1; i <= 5; i++) {
        char line[128];
        printf("Enter value #%d: ", i);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("No more input.\n");
            break;
        }
        /* Remove trailing newline if present */
        size_t n = strlen(line);
        if (n > 0 && line[n - 1] == '\n') {
            line[n - 1] = '\0';
        }

        ParseResult r = parse_int_strict(line);
        if (r.success) {
            printf("Parsed integer: %d\n", r.value);
        } else {
            printf("Error: %s\n", r.error);
        }
    }
    return 0;
}