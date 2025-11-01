#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static bool is_whitespace_but_not_newline(char c) {
    return c == ' ' || c == '\t' || c == '\v' || c == '\f';
}

static bool is_newline_char(char c) {
    return c == '\n' || c == '\r';
}

char* sanitize_argument_c(const char* input) {
    if (input == NULL) {
        return NULL;
    }

    size_t len = strlen(input);
    char* out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    memcpy(out, input, len);
    out[len] = '\0';

    size_t i = len;

    while (i > 0) {
        char c = out[i - 1];
        if (is_whitespace_but_not_newline(c)) {
            i--;
        } else {
            break;
        }
    }

    while (i > 0) {
        char c = out[i - 1];
        if (is_newline_char(c)) {
            i--;
        } else {
            break;
        }
    }

    out[i] = '\0'; /* null terminate */
    return out;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        char* res = sanitize_argument_c(argv[1]);
        if (res == NULL) {
            fputs("Error: Invalid input or memory allocation failed.\n", stderr);
            return 1;
        }
        fputs(res, stdout);
        fputc('\n', stdout);
        free(res);
    } else {
        /* 5 test cases */
        const char* tests[5] = {
            "Hello World   ",
            "Tabs and spaces\t \t   ",
            "Ends with newlines\n\n\r",
            "Mix \t\v\f\n\r",
            ""
        };
        for (int idx = 0; idx < 5; ++idx) {
            char* res = sanitize_argument_c(tests[idx]);
            if (res) {
                printf("[%s]\n", res);
                free(res);
            } else {
                fputs("Error\n", stderr);
            }
        }
    }
    return 0;
}