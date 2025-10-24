#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_PARTS 1000
#define MAX_PART_LEN 100000
#define MAX_TOTAL_LEN 1000000

static int read_line_alloc(char **out_str, size_t max_part_len) {
    if (out_str == NULL) return -1;
    *out_str = NULL;
    size_t buf_size = max_part_len + 2;
    char *buf = (char *)malloc(buf_size);
    if (!buf) {
        return -2;
    }
    if (fgets(buf, (int)buf_size, stdin) == NULL) {
        free(buf);
        return -1;
    }
    size_t len = strnlen(buf, buf_size);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else if (len == buf_size - 1 && buf[len - 1] != '\n') {
        int c;
        int too_long = 0;
        while ((c = getchar()) != '\n' && c != EOF) {
            too_long = 1;
        }
        free(buf);
        return too_long ? -3 : -1;
    }
    size_t final_len = strnlen(buf, buf_size);
    char *s = (char *)malloc(final_len + 1);
    if (!s) {
        free(buf);
        return -2;
    }
    memcpy(s, buf, final_len + 1);
    free(buf);
    *out_str = s;
    return 0;
}

char *safe_concat(const char *parts[], size_t count, size_t max_total_len, size_t max_part_len) {
    if (count > MAX_PARTS) {
        return NULL;
    }
    size_t total = 0;
    for (size_t i = 0; i < count; i++) {
        if (parts == NULL) {
            if (count == 0) break;
            return NULL;
        }
        if (parts[i] == NULL) {
            return NULL;
        }
        size_t len = strnlen(parts[i], max_part_len + 1);
        if (len > max_part_len) {
            return NULL;
        }
        if (len > max_total_len - total) {
            return NULL;
        }
        total += len;
    }
    char *out = (char *)malloc(total + 1);
    if (!out) {
        return NULL;
    }
    size_t offset = 0;
    for (size_t i = 0; i < count; i++) {
        if (parts == NULL) break;
        size_t len = strnlen(parts[i], max_part_len);
        memcpy(out + offset, parts[i], len);
        offset += len;
    }
    out[total] = '\0';
    return out;
}

static void run_tests(void) {
    const char *t1[] = {"Hello", " ", "World"};
    char *r1 = safe_concat(t1, 3, MAX_TOTAL_LEN, MAX_PART_LEN);
    printf("%s\n", r1 ? r1 : "ERROR");
    free(r1);

    const char *t2[] = {"", "abc", "", "def"};
    char *r2 = safe_concat(t2, 4, MAX_TOTAL_LEN, MAX_PART_LEN);
    printf("%s\n", r2 ? r2 : "ERROR");
    free(r2);

    const char *t3[] = {"multi", "-", "part", "-", "concat"};
    char *r3 = safe_concat(t3, 5, MAX_TOTAL_LEN, MAX_PART_LEN);
    printf("%s\n", r3 ? r3 : "ERROR");
    free(r3);

    const char **t4 = NULL; /* empty */
    char *r4 = safe_concat(t4, 0, MAX_TOTAL_LEN, MAX_PART_LEN);
    printf("%s\n", r4 ? r4 : "ERROR");
    free(r4);

    const char *t5[] = {"Safe ", "concat ", "test"};
    char *r5 = safe_concat(t5, 3, MAX_TOTAL_LEN, MAX_PART_LEN);
    printf("%s\n", r5 ? r5 : "ERROR");
    free(r5);
}

static int parse_int(const char *s, int *out) {
    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s) return 0;
    while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') end++;
    if (*end != '\0') return 0;
    if (v < 0 || v > INT_MAX) return 0;
    *out = (int)v;
    return 1;
}

static void read_from_user(void) {
    printf("Enter number of strings to concatenate (0..%d):\n", MAX_PARTS);
    char line[64];
    if (fgets(line, (int)sizeof(line), stdin) == NULL) {
        return;
    }
    int n = 0;
    if (!parse_int(line, &n) || n < 0 || n > (int)MAX_PARTS) {
        fprintf(stderr, "ERROR: Invalid count.\n");
        return;
    }
    if (n == 0) {
        printf("Concatenated result:\n\n");
        return;
    }
    char **parts = (char **)calloc((size_t)n, sizeof(char *));
    if (!parts) {
        fprintf(stderr, "ERROR: Memory allocation failed.\n");
        return;
    }
    size_t total = 0;
    for (int i = 0; i < n; i++) {
        char *s = NULL;
        int rc = read_line_alloc(&s, MAX_PART_LEN);
        if (rc != 0) {
            fprintf(stderr, "ERROR: Failed to read line or line too long.\n");
            for (int j = 0; j < i; j++) {
                free(parts[j]);
            }
            free(parts);
            return;
        }
        size_t len = strnlen(s, MAX_PART_LEN + 1);
        if (len > MAX_PART_LEN || len > (MAX_TOTAL_LEN - total)) {
            fprintf(stderr, "ERROR: Input exceeded allowed limits.\n");
            free(s);
            for (int j = 0; j < i; j++) {
                free(parts[j]);
            }
            free(parts);
            return;
        }
        total += len;
        parts[i] = s;
    }
    char *result = safe_concat((const char **)parts, (size_t)n, MAX_TOTAL_LEN, MAX_PART_LEN);
    if (!result) {
        fprintf(stderr, "ERROR: Concatenation failed due to invalid input or limits.\n");
        for (int i = 0; i < n; i++) free(parts[i]);
        free(parts);
        return;
    }
    printf("Concatenated result:\n%s\n", result);
    free(result);
    for (int i = 0; i < n; i++) free(parts[i]);
    free(parts);
}

int main(void) {
    run_tests();
    read_from_user();
    return 0;
}