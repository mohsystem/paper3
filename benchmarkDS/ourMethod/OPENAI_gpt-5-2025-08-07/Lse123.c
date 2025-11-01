#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_INPUT_SIZE (1u << 20) /* 1 MiB */

static bool is_space_nonnewline(char c) {
    return (c == ' ' || c == '\t' || c == '\v' || c == '\f');
}

static bool is_newline(char c) {
    return (c == '\n' || c == '\r');
}

/*
 * process_args:
 *  - Ensures exactly one argument is provided.
 *  - Validates that the argument pointer is non-NULL (best-effort "string" check in C).
 *  - Copies into 'out' safely after length checks.
 *  - Trims trailing whitespace (excluding CR/LF) in the first loop.
 *  - Trims trailing newlines in the second loop.
 *  - Null-terminates the result.
 * Returns 0 on success, non-zero on failure.
 */
int process_args(const char *const *args, size_t argc, char *out, size_t out_size) {
    if (out == NULL || out_size == 0) {
        return -1; /* invalid output buffer */
    }
    if (args == NULL || argc != 1) {
        return -2; /* invalid args */
    }
    const char *s = args[0];
    if (s == NULL) {
        return -3; /* invalid arg */
    }

    size_t in_len = strlen(s);
    if (in_len > MAX_INPUT_SIZE) {
        return -4; /* too long */
    }
    if (in_len >= out_size) {
        return -5; /* output buffer too small */
    }

    /* Copy input to output buffer for in-place trimming */
    memcpy(out, s, in_len);
    out[in_len] = '\0';

    /* First loop: trim trailing whitespace excluding CR/LF */
    size_t end = in_len;
    while (end > 0 && is_space_nonnewline(out[end - 1])) {
        end--;
    }

    /* Second loop: trim trailing newlines */
    while (end > 0 && is_newline(out[end - 1])) {
        end--;
    }

    /* Last line: null terminate the string */
    out[end] = '\0';
    return 0;
}

int main(void) {
    char buffer[256];

    const char *t1[] = { "hello   " };
    const char *t2[] = { "world\t\t\n" };
    const char *t3[] = { "line with spaces \r\n\n" };
    const char *t4[] = { "no-trim" };
    const char *t5[] = { "invalid", "extra" };

    const char *const *tests[] = { t1, t2, t3, t4, t5 };
    size_t test_argc[] = { 1, 1, 1, 1, 2 };

    for (size_t i = 0; i < 5; ++i) {
        int rc = process_args(tests[i], test_argc[i], buffer, sizeof(buffer));
        if (rc == 0) {
            printf("Test %zu -> [%s] len=%zu\n", i + 1, buffer, strlen(buffer));
        } else {
            printf("Test %zu -> error\n", i + 1);
        }
    }

    return 0;
}