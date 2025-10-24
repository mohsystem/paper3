#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

// Converts string to int; returns the value and sets *success to 1 if valid, else 0
int convert_to_int(const char* s, int* success) {
    if (!s) {
        if (success) *success = 0;
        return 0;
    }

    // Skip leading whitespace
    while (isspace((unsigned char)*s)) s++;

    if (*s == '\0') {
        if (success) *success = 0;
        return 0;
    }

    errno = 0;
    char* endptr = NULL;
    long val = strtol(s, &endptr, 10);

    // Skip trailing whitespace after number
    while (isspace((unsigned char)*endptr)) endptr++;

    if (errno == ERANGE || *endptr != '\0' || val < INT_MIN || val > INT_MAX) {
        if (success) *success = 0;
        return 0;
    }
    if (success) *success = 1;
    return (int)val;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            int ok = 0;
            int v = convert_to_int(argv[i], &ok);
            if (ok) {
                printf("Input: \"%s\" -> %d\n", argv[i], v);
            } else {
                printf("Input: \"%s\" -> invalid\n", argv[i]);
            }
        }
    } else {
        const char* tests[5] = {"123", "  42 ", "-7", "abc", "2147483648"};
        for (int i = 0; i < 5; ++i) {
            int ok = 0;
            int v = convert_to_int(tests[i], &ok);
            if (ok) {
                printf("Input: \"%s\" -> %d\n", tests[i], v);
            } else {
                printf("Input: \"%s\" -> invalid\n", tests[i]);
            }
        }
    }
    return 0;
}