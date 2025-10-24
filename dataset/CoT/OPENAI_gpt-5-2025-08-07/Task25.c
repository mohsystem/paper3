#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static void free_lines_array(char **arr, size_t n) {
    if (!arr) return;
    for (size_t i = 0; i < n; ++i) {
        free(arr[i]);
    }
    free(arr);
}

char **number_lines(const char *const *lines, size_t n) {
    if (n == 0) {
        return NULL;
    }
    if (lines == NULL) {
        return NULL;
    }

    char **out = (char **)calloc(n, sizeof(char *));
    if (!out) {
        return NULL;
    }

    for (size_t i = 0; i < n; ++i) {
        const char *s = lines[i] ? lines[i] : "null";
        size_t line_len = strlen(s);

        char numbuf[32];
        int numlen = snprintf(numbuf, sizeof(numbuf), "%zu", i + 1);
        if (numlen < 0) {
            free_lines_array(out, i);
            return NULL;
        }

        size_t digits = (size_t)numlen;
        size_t total_len;

        if (digits > SIZE_MAX - 2) { // for ": "
            free_lines_array(out, i);
            return NULL;
        }
        size_t prefix_len = digits + 2; // ": "
        if (prefix_len > SIZE_MAX - line_len) {
            free_lines_array(out, i);
            return NULL;
        }
        total_len = prefix_len + line_len;

        if (total_len > SIZE_MAX - 1) {
            free_lines_array(out, i);
            return NULL;
        }

        char *line = (char *)malloc(total_len + 1);
        if (!line) {
            free_lines_array(out, i);
            return NULL;
        }

        int written = snprintf(line, total_len + 1, "%s: %s", numbuf, s);
        if (written < 0 || (size_t)written != total_len) {
            free(line);
            free_lines_array(out, i);
            return NULL;
        }

        out[i] = line;
    }

    return out;
}

static void print_lines_array(char **arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        printf("\"%s\"", arr[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]\n");
}

int main(void) {
    // Test case 1: Empty array
    const char *t1[] = {};
    char **r1 = number_lines(t1, 0);
    print_lines_array(r1, 0);
    free_lines_array(r1, 0);

    // Test case 2: {"a", "b", "c"}
    const char *t2[] = {"a", "b", "c"};
    char **r2 = number_lines(t2, 3);
    print_lines_array(r2, 3);
    free_lines_array(r2, 3);

    // Test case 3: {"", "foo", "bar"}
    const char *t3[] = {"", "foo", "bar"};
    char **r3 = number_lines(t3, 3);
    print_lines_array(r3, 3);
    free_lines_array(r3, 3);

    // Test case 4: {"hello world", "Line 2"}
    const char *t4[] = {"hello world", "Line 2"};
    char **r4 = number_lines(t4, 2);
    print_lines_array(r4, 2);
    free_lines_array(r4, 2);

    // Test case 5: {"multiple", "lines", "with", "numbers", "10"}
    const char *t5[] = {"multiple", "lines", "with", "numbers", "10"};
    char **r5 = number_lines(t5, 5);
    print_lines_array(r5, 5);
    free_lines_array(r5, 5);

    return 0;
}