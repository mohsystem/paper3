#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    char **lines;
    size_t count;
} MatchResult;

static void free_match_result(MatchResult *res) {
    if (!res) return;
    if (res->lines) {
        for (size_t i = 0; i < res->count; ++i) {
            free(res->lines[i]);
        }
        free(res->lines);
        res->lines = NULL;
    }
    res->count = 0;
}

MatchResult match_lines_start_num_end_dot(const char *file_path) {
    MatchResult result = {0};
    if (file_path == NULL) {
        return result;
    }

    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        return result;
    }

    regex_t regex;
    const char *pattern = "^[0-9]+.*\\.$";
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fclose(fp);
        return result;
    }

    char *line = NULL;
    size_t cap = 0;
    ssize_t n;

    while ((n = getline(&line, &cap, fp)) != -1) {
        // Strip trailing CR/LF
        while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
            line[--n] = '\0';
        }

        int rc = regexec(&regex, line, 0, NULL, 0);
        if (rc == 0) {
            char *copy = strdup(line);
            if (!copy) {
                free_match_result(&result);
                break;
            }
            char **newarr = (char **)realloc(result.lines, (result.count + 1) * sizeof(char *));
            if (!newarr) {
                free(copy);
                free_match_result(&result);
                break;
            }
            result.lines = newarr;
            result.lines[result.count++] = copy;
        }
    }

    free(line);
    regfree(&regex);
    fclose(fp);
    return result;
}

static int write_lines_to_fd(int fd, const char *const *lines, size_t count) {
    FILE *fp = fdopen(fd, "wb");
    if (!fp) return -1;
    for (size_t i = 0; i < count; ++i) {
        if (fputs(lines[i], fp) == EOF || fputc('\n', fp) == EOF) {
            fclose(fp);
            return -1;
        }
    }
    if (fflush(fp) != 0) {
        fclose(fp);
        return -1;
    }
    // Do not close underlying fd twice; fp takes ownership of fd.
    if (fclose(fp) != 0) {
        return -1;
    }
    return 0;
}

static char *create_temp_file_with_lines(const char *const *lines, size_t count) {
    char tmpl[] = "/tmp/task99_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        return NULL;
    }
    if (write_lines_to_fd(fd, lines, count) != 0) {
        unlink(tmpl);
        return NULL;
    }
    // Return a heap-allocated copy of the path
    char *path = strdup(tmpl);
    if (!path) {
        unlink(tmpl);
        return NULL;
    }
    return path;
}

static void print_test(const char *title, MatchResult *res) {
    printf("%s\n", title);
    for (size_t i = 0; i < res->count; ++i) {
        printf("%s\n", res->lines[i]);
    }
    printf("---\n");
}

int main(void) {
    // Test 1
    const char *t1[] = { "123.", "1abc.", "abc.", "123", " 123.", "456.." };
    char *f1 = create_temp_file_with_lines(t1, sizeof(t1)/sizeof(t1[0]));
    MatchResult r1 = match_lines_start_num_end_dot(f1);
    print_test("Test 1", &r1);
    free_match_result(&r1);
    if (f1) { unlink(f1); free(f1); }

    // Test 2: empty file
    const char *t2[] = { };
    char *f2 = create_temp_file_with_lines(t2, 0);
    MatchResult r2 = match_lines_start_num_end_dot(f2);
    print_test("Test 2", &r2);
    free_match_result(&r2);
    if (f2) { unlink(f2); free(f2); }

    // Test 3: whitespace and numbers
    const char *t3[] = { "\t789.", "000.", "000. ", "999.x", "42." };
    char *f3 = create_temp_file_with_lines(t3, sizeof(t3)/sizeof(t3[0]));
    MatchResult r3 = match_lines_start_num_end_dot(f3);
    print_test("Test 3", &r3);
    free_match_result(&r3);
    if (f3) { unlink(f3); free(f3); }

    // Test 4: mixed punctuation
    const char *t4[] = { "7!@$%.", "5text.", "0042..", "nope.", "9" };
    char *f4 = create_temp_file_with_lines(t4, sizeof(t4)/sizeof(t4[0]));
    MatchResult r4 = match_lines_start_num_end_dot(f4);
    print_test("Test 4", &r4);
    free_match_result(&r4);
    if (f4) { unlink(f4); free(f4); }

    // Test 5: long lines and edge cases
    char *long_line = (char *)malloc(1001);
    if (!long_line) return 1;
    memset(long_line, 'x', 1000);
    long_line[1000] = '\0';
    char *ll_with = (char *)malloc(1003);
    if (!ll_with) { free(long_line); return 1; }
    snprintf(ll_with, 1003, "1%s.", long_line);
    char *ll_only = (char *)malloc(1002);
    if (!ll_only) { free(long_line); free(ll_with); return 1; }
    snprintf(ll_only, 1002, "%s.", long_line);
    const char *t5[] = { ll_with, ll_only, "2.", "3. ", "4.." };
    char *f5 = create_temp_file_with_lines(t5, sizeof(t5)/sizeof(t5[0]));
    MatchResult r5 = match_lines_start_num_end_dot(f5);
    print_test("Test 5", &r5);
    free_match_result(&r5);
    if (f5) { unlink(f5); free(f5); }
    free(long_line);
    free(ll_with);
    free(ll_only);

    return 0;
}