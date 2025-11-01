#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char** lines;
    int count;
} Output;

static char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* p = (char*)malloc(len + 1);
    if (p) memcpy(p, s, len + 1);
    return p;
}

static char* rtrim_copy(const char* s) {
    if (!s) return my_strdup("");
    size_t len = strlen(s);
    if (len == 0) return my_strdup("");
    size_t end = len;
    while (end > 0 && isspace((unsigned char)s[end - 1])) {
        end--;
    }
    char* out = (char*)malloc(end + 1);
    if (!out) return NULL;
    memcpy(out, s, end);
    out[end] = '\0';
    return out;
}

Output run(int argc, const char* argv[]) {
    Output out;
    out.lines = NULL;
    out.count = 0;

    if (argc != 1) {
        out.count = 1;
        out.lines = (char**)malloc(sizeof(char*) * out.count);
        if (out.lines) {
            out.lines[0] = my_strdup("Error: expected exactly one argument.");
        }
        return out;
    }

    const char* input = argv[0];

    out.count = 2;
    out.lines = (char**)malloc(sizeof(char*) * out.count);
    if (!out.lines) {
        out.count = 0;
        return out;
    }
    out.lines[0] = my_strdup(input);
    out.lines[1] = rtrim_copy(input);

    return out;
}

int main(void) {
    // Test 1: no args
    Output o1 = run(0, NULL);

    // Test 2: one arg, no trailing whitespace
    const char* a2[] = {"hello"};
    Output o2 = run(1, a2);

    // Test 3: one arg with trailing spaces
    const char* a3[] = {"hello   "};
    Output o3 = run(1, a3);

    // Test 4: one arg with trailing tabs and spaces
    const char* a4[] = {"world\t \t"};
    Output o4 = run(1, a4);

    // Test 5: more than one arg
    const char* a5[] = {"a", "b"};
    Output o5 = run(2, a5);

    Output tests[] = {o1, o2, o3, o4, o5};
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        for (int j = 0; j < tests[i].count; ++j) {
            if (tests[i].lines && tests[i].lines[j]) {
                printf("%s\n", tests[i].lines[j]);
                free(tests[i].lines[j]);
            }
        }
        free(tests[i].lines);
    }
    return 0;
}