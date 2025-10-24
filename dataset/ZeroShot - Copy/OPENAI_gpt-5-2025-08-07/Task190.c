#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void free_string_array(char **arr, size_t n) {
    if (!arr) return;
    for (size_t i = 0; i < n; ++i) {
        free(arr[i]);
    }
    free(arr);
}

char **transpose_text(const char *const *lines, size_t n_lines, size_t *out_rows) {
    if (out_rows) *out_rows = 0;
    if (lines == NULL || n_lines == 0) return NULL;

    char ***matrix = (char ***)calloc(n_lines, sizeof(char **));
    size_t *counts = (size_t *)calloc(n_lines, sizeof(size_t));
    if (!matrix || !counts) {
        free(matrix);
        free(counts);
        return NULL;
    }

    size_t cols = 0;
    for (size_t i = 0; i < n_lines; ++i) {
        const char *s = lines[i];
        if (s == NULL) {
            // cleanup
            for (size_t r = 0; r < i; ++r) {
                for (size_t c = 0; c < counts[r]; ++c) free(matrix[r][c]);
                free(matrix[r]);
            }
            free(matrix);
            free(counts);
            return NULL;
        }

        size_t cap = 8, cnt = 0;
        char **tokens = (char **)malloc(cap * sizeof(char *));
        if (!tokens) {
            for (size_t r = 0; r < i; ++r) {
                for (size_t c = 0; c < counts[r]; ++c) free(matrix[r][c]);
                free(matrix[r]);
            }
            free(matrix);
            free(counts);
            return NULL;
        }

        size_t pos = 0;
        while (s[pos] != '\0') {
            while (s[pos] != '\0' && isspace((unsigned char)s[pos])) pos++;
            if (s[pos] == '\0') break;
            size_t start = pos;
            while (s[pos] != '\0' && !isspace((unsigned char)s[pos])) pos++;
            size_t len = pos - start;
            char *tok = (char *)malloc(len + 1);
            if (!tok) {
                for (size_t k = 0; k < cnt; ++k) free(tokens[k]);
                free(tokens);
                for (size_t r = 0; r < i; ++r) {
                    for (size_t c = 0; c < counts[r]; ++c) free(matrix[r][c]);
                    free(matrix[r]);
                }
                free(matrix);
                free(counts);
                return NULL;
            }
            memcpy(tok, s + start, len);
            tok[len] = '\0';

            if (cnt == cap) {
                size_t ncap = cap * 2;
                char **tmp = (char **)realloc(tokens, ncap * sizeof(char *));
                if (!tmp) {
                    free(tok);
                    for (size_t k = 0; k < cnt; ++k) free(tokens[k]);
                    free(tokens);
                    for (size_t r = 0; r < i; ++r) {
                        for (size_t c = 0; c < counts[r]; ++c) free(matrix[r][c]);
                        free(matrix[r]);
                    }
                    free(matrix);
                    free(counts);
                    return NULL;
                }
                tokens = tmp;
                cap = ncap;
            }
            tokens[cnt++] = tok;
        }

        if (i == 0) {
            cols = cnt;
            if (cols == 0) {
                // cleanup tokens
                free_string_array(tokens, cnt);
                free(matrix);
                free(counts);
                return NULL;
            }
        } else {
            if (cnt != cols) {
                // cleanup tokens and previous rows
                free_string_array(tokens, cnt);
                for (size_t r = 0; r < i; ++r) {
                    for (size_t c = 0; c < counts[r]; ++c) free(matrix[r][c]);
                    free(matrix[r]);
                }
                free(matrix);
                free(counts);
                return NULL;
            }
        }

        matrix[i] = tokens;
        counts[i] = cnt;
    }

    if (cols == 0) {
        for (size_t r = 0; r < n_lines; ++r) {
            free(matrix[r]);
        }
        free(matrix);
        free(counts);
        return NULL;
    }

    char **out = (char **)malloc(cols * sizeof(char *));
    if (!out) {
        for (size_t r = 0; r < n_lines; ++r) {
            for (size_t c = 0; c < counts[r]; ++c) free(matrix[r][c]);
            free(matrix[r]);
        }
        free(matrix);
        free(counts);
        return NULL;
    }

    for (size_t c = 0; c < cols; ++c) {
        size_t total = 0;
        for (size_t r = 0; r < n_lines; ++r) {
            total += strlen(matrix[r][c]);
        }
        if (n_lines > 0) total += (n_lines - 1); // spaces
        char *row = (char *)malloc(total + 1);
        if (!row) {
            for (size_t k = 0; k < c; ++k) free(out[k]);
            free(out);
            for (size_t r = 0; r < n_lines; ++r) {
                for (size_t j = 0; j < counts[r]; ++j) free(matrix[r][j]);
                free(matrix[r]);
            }
            free(matrix);
            free(counts);
            return NULL;
        }
        size_t offset = 0;
        for (size_t r = 0; r < n_lines; ++r) {
            size_t len = strlen(matrix[r][c]);
            memcpy(row + offset, matrix[r][c], len);
            offset += len;
            if (r + 1 < n_lines) {
                row[offset++] = ' ';
            }
        }
        row[offset] = '\0';
        out[c] = row;
    }

    for (size_t r = 0; r < n_lines; ++r) {
        for (size_t j = 0; j < counts[r]; ++j) free(matrix[r][j]);
        free(matrix[r]);
    }
    free(matrix);
    free(counts);

    if (out_rows) *out_rows = cols;
    return out;
}

static void print_and_free_result(char **res, size_t rows) {
    if (!res) {
        printf("NULL\n---\n");
        return;
    }
    for (size_t i = 0; i < rows; ++i) {
        printf("%s\n", res[i]);
        free(res[i]);
    }
    free(res);
    printf("---\n");
}

int main(void) {
    // Test 1: Sample
    const char *t1[] = {
        "name age",
        "alice 21",
        "ryan 30"
    };
    size_t rows = 0;
    char **r1 = transpose_text(t1, 3, &rows);
    printf("Test 1 output:\n");
    print_and_free_result(r1, rows);

    // Test 2: Single row
    const char *t2[] = { "a b c" };
    rows = 0;
    char **r2 = transpose_text(t2, 1, &rows);
    printf("Test 2 output:\n");
    print_and_free_result(r2, rows);

    // Test 3: Single column
    const char *t3[] = { "a", "b", "c" };
    rows = 0;
    char **r3 = transpose_text(t3, 3, &rows);
    printf("Test 3 output:\n");
    print_and_free_result(r3, rows);

    // Test 4: Multiple spaces and trims
    const char *t4[] = { "x   y", " 1    2 " };
    rows = 0;
    char **r4 = transpose_text(t4, 2, &rows);
    printf("Test 4 output:\n");
    print_and_free_result(r4, rows);

    // Test 5: 2x2
    const char *t5[] = { "p q", "r s" };
    rows = 0;
    char **r5 = transpose_text(t5, 2, &rows);
    printf("Test 5 output:\n");
    print_and_free_result(r5, rows);

    return 0;
}