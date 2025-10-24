#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

int try_parse_int(const char* s, int* out) {
    if (s == NULL || out == NULL) return 0;

    // Skip leading whitespace
    while (isspace((unsigned char)*s)) s++;

    if (*s == '\0') return 0; // empty or only spaces

    errno = 0;
    char* endptr = NULL;
    long val = strtol(s, &endptr, 10);

    if (endptr == s) return 0; // no digits

    // Skip trailing whitespace
    while (*endptr && isspace((unsigned char)*endptr)) endptr++;

    if (*endptr != '\0') return 0; // extra non-space chars
    if (errno == ERANGE) return 0; // out of range for long
    if (val < INT_MIN || val > INT_MAX) return 0;

    *out = (int)val;
    return 1;
}

int main(void) {
    char buffer[256];
    for (int i = 1; i <= 5; ++i) {
        printf("Enter value #%d: ", i);
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        // Remove trailing newline if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';

        int result;
        if (try_parse_int(buffer, &result)) {
            printf("Parsed integer: %d\n", result);
        } else {
            printf("Invalid integer\n");
        }
    }
    return 0;
}