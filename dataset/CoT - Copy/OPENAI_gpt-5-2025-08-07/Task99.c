#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

typedef struct {
    char **lines;
    size_t count;
} MatchResult;

static void free_match_result(MatchResult *res) {
    if (!res) return;
    for (size_t i = 0; i < res->count; ++i) {
        free(res->lines[i]);
    }
    free(res->lines);
    res->lines = NULL;
    res->count = 0;
}

static char* read_line_dynamic(FILE *fp) {
    if (!fp) return NULL;
    size_t cap = 256;
    size_t len = 0;
    char *buf = (char*)malloc(cap);
    if (!buf) return NULL;

    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (len + 1 >= cap) {
            size_t ncap = cap * 2;
            char *nbuf = (char*)realloc(buf, ncap);
            if (!nbuf) {
                free(buf);
                return NULL;
            }
            buf = nbuf;
            cap = ncap;
        }
        buf[len++] = (char)c;
        if (c == '\n') break;
    }

    if (len == 0 && c == EOF) {
        free(buf);
        return NULL;
    }

    // Trim trailing newline
    if (len > 0 && buf[len - 1] == '\n') {
        len--;
    }
    // Trim trailing CR (Windows line endings)
    if (len > 0 && buf[len - 1] == '\r') {
        len--;
    }

    buf[len] = '\0';
    return buf;
}

MatchResult match_lines_start_num_end_dot(const char *file_path) {
    MatchResult result;
    result.lines = NULL;
    result.count = 0;

    if (!file_path) {
        return result;
    }

    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        return result;
    }

    regex_t regex;
    const char *pattern = "^[0-9]+.*\\.$";
    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
        fclose(fp);
        return result;
    }

    size_t cap = 0;
    char *line = NULL;
    while ((line = read_line_dynamic(fp)) != NULL) {
        int reti = regexec(&regex, line, 0, NULL, 0);
        if (reti == 0) {
            // Match
            if (result.count + 1 > cap) {
                size_t ncap = cap == 0 ? 8 : cap * 2;
                char **narr = (char**)realloc(result.lines, ncap * sizeof(char*));
                if (!narr) {
                    free(line);
                    // On allocation error, stop processing further to avoid partial inconsistent state
                    break;
                }
                result.lines = narr;
                cap = ncap;
            }
            result.lines[result.count] = line;
            result.count += 1;
        } else {
            free(line);
        }
    }

    regfree(&regex);
    fclose(fp);
    return result;
}

static int write_file(const char *path, const char **lines, size_t n) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    for (size_t i = 0; i < n; ++i) {
        if (fputs(lines[i], fp) < 0) { fclose(fp); return 0; }
        if (fputc('\n', fp) == EOF) { fclose(fp); return 0; }
    }
    fclose(fp);
    return 1;
}

static void print_result(const char *title, MatchResult *res) {
    printf("%s\n", title);
    for (size_t i = 0; i < res->count; ++i) {
        printf("%s\n", res->lines[i]);
    }
    printf("----\n");
}

int main(void) {
    // Test case 1
    const char *f1 = "task99_c_test1.txt";
    const char *l1[] = {
        "123 This line starts with numbers and ends with a dot.",
        "Not starting with number.",
        "42.",
        "7 ends without dot",
        "007 Bond."
    };
    write_file(f1, l1, sizeof(l1)/sizeof(l1[0]));
    MatchResult r1 = match_lines_start_num_end_dot(f1);
    print_result("Test 1:", &r1);
    free_match_result(&r1);

    // Test case 2
    const char *f2 = "task99_c_test2.txt";
    const char *l2[] = {
        "1.",
        "22.",
        "333.",
        "4444."
    };
    write_file(f2, l2, sizeof(l2)/sizeof(l2[0]));
    MatchResult r2 = match_lines_start_num_end_dot(f2);
    print_result("Test 2:", &r2);
    free_match_result(&r2);

    // Test case 3
    const char *f3 = "task99_c_test3.txt";
    const char *l3[] = {
        "hello.",
        ".",
        " no number.",
        "123 but no dot at end",
        ""
    };
    write_file(f3, l3, sizeof(l3)/sizeof(l3[0]));
    MatchResult r3 = match_lines_start_num_end_dot(f3);
    print_result("Test 3:", &r3);
    free_match_result(&r3);

    // Test case 4
    const char *f4 = "task99_c_test4.txt";
    const char *l4[] = {
        "0.",
        "9 Ends without dot",
        "123abc.",
        "999!.",
        "001 leading zeros."
    };
    write_file(f4, l4, sizeof(l4)/sizeof(l4[0]));
    MatchResult r4 = match_lines_start_num_end_dot(f4);
    print_result("Test 4:", &r4);
    free_match_result(&r4);

    // Test case 5
    const char *f5 = "task99_c_test5.txt";
    const char *l5[] = {
        "123.456.",
        "10. end.",
        "5... ... ...",
        "8.",
        "12.3"
    };
    write_file(f5, l5, sizeof(l5)/sizeof(l5[0]));
    MatchResult r5 = match_lines_start_num_end_dot(f5);
    print_result("Test 5:", &r5);
    free_match_result(&r5);

    return 0;
}