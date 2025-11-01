#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char **lines;
    size_t count;
    int exit_code;
} Result;

static void free_result(Result *r) {
    if (!r) return;
    if (r->lines) {
        for (size_t i = 0; i < r->count; ++i) {
            free(r->lines[i]);
        }
        free(r->lines);
        r->lines = NULL;
    }
    r->count = 0;
}

static char* safe_strdup(const char* s) {
    if (s == NULL) {
        char *z = (char*)calloc(1, 1);
        return z;
    }
    size_t len = strnlen(s, 1ULL << 20); // cap to 1MB to avoid pathological inputs
    char *copy = (char*)malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, s, len);
    copy[len] = '\0';
    return copy;
}

static char* rtrim_copy(const char* s) {
    if (s == NULL) {
        char *z = (char*)calloc(1, 1);
        return z;
    }
    size_t len = strnlen(s, 1ULL << 20);
    char *buf = (char*)malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, s, len);
    buf[len] = '\0';
    if (len == 0) return buf;
    size_t end = len;
    while (end > 0 && isspace((unsigned char)buf[end - 1])) {
        end--;
    }
    buf[end] = '\0';
    return buf;
}

// Function that accepts input as parameters and returns the output (and exit code)
static Result process_args(int argc, const char* argv[]) {
    Result res;
    res.lines = NULL;
    res.count = 0;
    res.exit_code = 0;

    if (argc != 1) {
        res.lines = (char**)calloc(1, sizeof(char*));
        if (!res.lines) {
            // Allocation failure fallback: print directly and set exit code
            fprintf(stderr, "Error: expected exactly one argument.\n");
            res.exit_code = 1;
            return res;
        }
        res.lines[0] = safe_strdup("Error: expected exactly one argument.");
        if (!res.lines[0]) {
            free(res.lines);
            res.lines = NULL;
            fprintf(stderr, "Error: memory allocation failed.\n");
            res.exit_code = 1;
            return res;
        }
        res.count = 1;
        res.exit_code = 1;
        return res;
    }

    const char* input = (argv && argv[0]) ? argv[0] : "";
    res.lines = (char**)calloc(2, sizeof(char*));
    if (!res.lines) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        res.exit_code = 1;
        return res;
    }

    res.lines[0] = safe_strdup(input);
    if (!res.lines[0]) {
        free(res.lines);
        res.lines = NULL;
        fprintf(stderr, "Error: memory allocation failed.\n");
        res.exit_code = 1;
        return res;
    }

    res.lines[1] = rtrim_copy(input);
    if (!res.lines[1]) {
        free(res.lines[0]);
        free(res.lines);
        res.lines = NULL;
        fprintf(stderr, "Error: memory allocation failed.\n");
        res.exit_code = 1;
        return res;
    }

    res.count = 2;
    res.exit_code = 0;
    return res;
}

static void print_result(const Result *r) {
    if (!r || !r->lines) return;
    for (size_t i = 0; i < r->count; ++i) {
        if (r->lines[i]) {
            puts(r->lines[i]);
        } else {
            puts("");
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        Result r = process_args(argc - 1, (const char**)&argv[1]);
        print_result(&r);
        int code = r.exit_code;
        free_result(&r);
        return code;
    } else {
        // 5 test cases
        const char* t1_args[] = { };
        const char* t2_args[] = { "hello" };
        const char* t3_args[] = { "abc   " };
        const char* t4_args[] = { "foo \t" };
        const char* t5_args[] = { "one", "two" };

        struct {
            const char** args;
            int count;
        } tests[] = {
            { t1_args, 0 },
            { t2_args, 1 },
            { t3_args, 1 },
            { t4_args, 1 },
            { t5_args, 2 }
        };

        for (int i = 0; i < 5; ++i) {
            printf("=== Test %d ===\n", i + 1);
            Result r = process_args(tests[i].count, tests[i].args);
            print_result(&r);
            printf("Exit code: %d\n", r.exit_code);
            free_result(&r);
        }
    }
    return 0;
}