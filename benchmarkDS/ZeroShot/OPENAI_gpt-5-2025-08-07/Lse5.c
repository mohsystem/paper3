#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

int get_element_at_index(const char* arr[], size_t len, const char* index_str, const char** out_element) {
    if (arr == NULL || index_str == NULL || out_element == NULL) {
        return -1; // invalid parameters
    }

    // Trim leading/trailing whitespace
    const char* s = index_str;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' || *s == '\f' || *s == '\v') s++;
    size_t slen = strlen(s);
    while (slen > 0 && (s[slen - 1] == ' ' || s[slen - 1] == '\t' || s[slen - 1] == '\n' || s[slen - 1] == '\r' || s[slen - 1] == '\f' || s[slen - 1] == '\v')) {
        slen--;
    }
    if (slen == 0) return -2; // empty after trim

    // Create a temporary null-terminated trimmed copy
    char buf[128];
    if (slen >= sizeof(buf)) return -3; // avoid overly long input
    memcpy(buf, s, slen);
    buf[slen] = '\0';

    // Disallow negative numbers
    if (buf[0] == '-') return -4;
    if (buf[0] == '+' && buf[1] == '\0') return -5; // only '+' is invalid

    errno = 0;
    char* endptr = NULL;
    long val = strtol(buf, &endptr, 10);
    if (errno == ERANGE) return -6;
    if (endptr == buf || *endptr != '\0') return -7; // not a pure integer
    if (val < 0) return -8;
    if (val > (long)SIZE_MAX) return -9;

    size_t idx = (size_t)val;
    if (idx >= len) return -10;

    *out_element = arr[idx];
    return 0;
}

int main(int argc, char* argv[]) {
    const char* elements[] = {"alpha", "bravo", "charlie", "delta", "echo"};
    size_t len = sizeof(elements) / sizeof(elements[0]);

    // Process command-line argument if provided
    if (argc >= 2) {
        const char* out = NULL;
        int rc = get_element_at_index(elements, len, argv[1], &out);
        if (rc == 0 && out != NULL) {
            printf("%s\n", out);
        } else {
            fputs("Invalid input or index out of bounds.\n", stderr);
        }
    }

    // Five test cases
    const char* tests[5] = {"0", "2", "4", "-1", "abc"};
    for (int i = 0; i < 5; ++i) {
        const char* out = NULL;
        int rc = get_element_at_index(elements, len, tests[i], &out);
        if (rc == 0 && out != NULL) {
            printf("Test index \"%s\": %s\n", tests[i], out);
        } else {
            printf("Test index \"%s\": Invalid\n", tests[i]);
        }
    }

    return 0;
}