#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* trim_trailing(const char* input) {
    if (input == NULL) return NULL;
    size_t len = strlen(input);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    strcpy(out, input);
    while (len > 0 && isspace((unsigned char)out[len - 1])) {
        out[len - 1] = '\0';
        len--;
    }
    return out;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        char* trimmed = trim_trailing(argv[1]);
        if (trimmed) {
            printf("%s\n", trimmed);
            free(trimmed);
        }
    } else {
        const char* tests[5] = {
            "hello   ",
            "test\t\t",
            "line\n",
            " mix  \t\n",
            "no-trim"
        };
        for (int i = 0; i < 5; i++) {
            char* trimmed = trim_trailing(tests[i]);
            if (trimmed) {
                printf("%s\n", trimmed);
                free(trimmed);
            }
        }
    }
    return 0;
}