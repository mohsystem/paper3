#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* trim_trailing_whitespace_copy(const char* s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = strlen(s);
    char* copy = (char*)malloc(len + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, s, len + 1); // include terminating null

    while (len > 0 && isspace((unsigned char)copy[len - 1])) {
        copy[len - 1] = '\0';
        --len;
    }
    return copy;
}

int main(int argc, char* argv[]) {
    // Command-line behavior
    if (argc > 1) {
        char* out = trim_trailing_whitespace_copy(argv[1]);
        if (out != NULL) {
            printf("%s\n", out);
            free(out);
        } else {
            printf("\n");
        }
    }

    // 5 test cases
    const char* tests[5] = {
        "hello   ",
        " no-trim",
        "tabs\t\t",
        "mixed spaces\t ",
        ""
    };

    for (size_t i = 0; i < 5; ++i) {
        char* res = trim_trailing_whitespace_copy(tests[i]);
        if (res != NULL) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("\n");
        }
    }
    return 0;
}