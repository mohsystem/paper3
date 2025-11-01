#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

char* trim_trailing_whitespace(const char* s) {
    const char* input = (s != NULL) ? s : "";
    size_t len = strlen(input);
    size_t end = len;

    while (end > 0 && isspace((unsigned char)input[end - 1])) {
        end--;
    }

    if (end > SIZE_MAX - 1) {
        return NULL; // size overflow guard (theoretically unreachable here)
    }

    char* out = (char*)malloc(end + 1);
    if (out == NULL) {
        return NULL;
    }

    if (end > 0) {
        memcpy(out, input, end);
    }
    out[end] = '\0';
    return out;
}

int main(int argc, char* argv[]) {
    // If a command-line argument is provided, trim trailing whitespace and print it.
    if (argc > 1 && argv[1] != NULL) {
        char* trimmed = trim_trailing_whitespace(argv[1]);
        if (trimmed != NULL) {
            printf("%s\n", trimmed);
            free(trimmed);
        }
    }

    // Five test cases
    const char* tests[] = {
        "hello",
        "hello   ",
        "line\n",
        "tabs\t\t",
        "mix \t\n\r"
    };

    size_t num_tests = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < num_tests; ++i) {
        char* res = trim_trailing_whitespace(tests[i]);
        if (res != NULL) {
            printf("TEST %zu: [%s]\n", i + 1, res);
            free(res);
        } else {
            printf("TEST %zu: [ERROR]\n", i + 1);
        }
    }

    return 0;
}