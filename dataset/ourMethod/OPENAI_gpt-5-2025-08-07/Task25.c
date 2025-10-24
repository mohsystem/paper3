#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
   Returns a newly allocated array of numbered lines.
   - lines: array of C strings (may contain NULL entries which are treated as empty).
   - count: number of input lines.
   - out_count: set to number of output lines on success (equals count). Set to 0 on error.
   On success: returns non-NULL if count > 0, or NULL if count == 0.
   On error: returns NULL and out_count is 0.
*/
char** number_lines(const char* const* lines, size_t count, size_t* out_count) {
    const size_t MAX_LINE_LEN = 1u << 20; /* 1 MiB limit per input line */
    if (out_count == NULL) {
        return NULL;
    }
    *out_count = 0;

    if (lines == NULL && count != 0) {
        return NULL;
    }
    if (count == 0) {
        return NULL; /* Represent empty list with NULL and out_count = 0 */
    }

    char** out = (char**)calloc(count, sizeof(char*));
    if (!out) {
        return NULL;
    }

    for (size_t i = 0; i < count; ++i) {
        const char* src = (lines && lines[i]) ? lines[i] : "";
        size_t len = 0;

        /* Safe bounded length check */
        const char* p = src;
        while (len < MAX_LINE_LEN && *p != '\0') {
            ++p;
            ++len;
        }
        if (len >= MAX_LINE_LEN && *p != '\0') {
            /* Exceeded permitted length */
            for (size_t j = 0; j < i; ++j) {
                free(out[j]);
            }
            free(out);
            return NULL;
        }

        /* Count digits of (i+1) */
        size_t n = i + 1;
        size_t digits = 0;
        while (n > 0) {
            n /= 10;
            ++digits;
        }
        if (digits == 0) digits = 1;

        /* Compute total size: digits + 2 (": ") + len + 1 (NUL) */
        size_t total = digits;
        if (total > SIZE_MAX - 2) { /* overflow check */
            for (size_t j = 0; j < i; ++j) free(out[j]);
            free(out);
            return NULL;
        }
        total += 2;
        if (total > SIZE_MAX - len) {
            for (size_t j = 0; j < i; ++j) free(out[j]);
            free(out);
            return NULL;
        }
        total += len;
        if (total == SIZE_MAX) {
            for (size_t j = 0; j < i; ++j) free(out[j]);
            free(out);
            return NULL;
        }
        total += 1; /* for '\0' */

        char* buf = (char*)malloc(total);
        if (!buf) {
            for (size_t j = 0; j < i; ++j) free(out[j]);
            free(out);
            return NULL;
        }

        /* Safe formatting */
        int written = snprintf(buf, total, "%zu: %s", i + 1, src);
        if (written < 0 || (size_t)written >= total) {
            free(buf);
            for (size_t j = 0; j < i; ++j) free(out[j]);
            free(out);
            return NULL;
        }

        out[i] = buf;
    }

    *out_count = count;
    return out;
}

static void print_lines(char* const* lines, size_t count) {
    printf("[");
    for (size_t i = 0; i < count; ++i) {
        if (lines && lines[i]) {
            printf("\"%s\"", lines[i]);
        } else {
            printf("\"\"");
        }
        if (i + 1 < count) printf(", ");
    }
    printf("]\n");
}

static void free_lines(char** lines, size_t count) {
    if (!lines) return;
    for (size_t i = 0; i < count; ++i) {
        free(lines[i]);
    }
    free(lines);
}

int main(void) {
    /* Test 1: Empty input */
    {
        size_t outc = 0;
        char** out = number_lines(NULL, 0, &outc);
        print_lines(out, outc);
        free_lines(out, outc);
    }

    /* Test 2: Basic example */
    {
        const char* in[] = {"a", "b", "c"};
        size_t outc = 0;
        char** out = number_lines(in, 3, &outc);
        print_lines(out, outc);
        free_lines(out, outc);
    }

    /* Test 3: Contains empty strings and NULL entry */
    {
        const char* in[] = {"", "line", NULL, "end"};
        size_t outc = 0;
        char** out = number_lines(in, 4, &outc);
        print_lines(out, outc);
        free_lines(out, outc);
    }

    /* Test 4: Mixed punctuation and spaces */
    {
        const char* in[] = {"Hello, world!", "  leading space", "trailing space  ", "colon: test"};
        size_t outc = 0;
        char** out = number_lines(in, 4, &outc);
        print_lines(out, outc);
        free_lines(out, outc);
    }

    /* Test 5: Long string */
    {
        size_t L = 2000;
        char* longStr = (char*)malloc(L + 1);
        if (!longStr) {
            fprintf(stderr, "Allocation failed\n");
            return 1;
        }
        memset(longStr, 'x', L);
        longStr[L] = '\0';
        const char* in[] = {longStr, "short", "end"};
        size_t outc = 0;
        char** out = number_lines(in, 3, &outc);
        if (out == NULL && outc == 0) {
            fprintf(stderr, "number_lines failed\n");
            free(longStr);
            return 1;
        }
        /* Print lengths to keep console readable */
        printf("[");
        for (size_t i = 0; i < outc; ++i) {
            size_t len = out[i] ? strlen(out[i]) : 0;
            printf("\"len=%zu\"", len);
            if (i + 1 < outc) printf(", ");
        }
        printf("]\n");

        free_lines(out, outc);
        free(longStr);
    }

    return 0;
}