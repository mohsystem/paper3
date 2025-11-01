#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char* trim_trailing(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        len--;
    }
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
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
            "Hello World   ",
            "TrailingTabs\t\t\t",
            "LineEnd\n\n",
            "  LeadingShouldStay   ",
            "    "
        };
        for (int i = 0; i < 5; ++i) {
            char* trimmed = trim_trailing(tests[i]);
            if (trimmed) {
                printf("%s\n", trimmed);
                free(trimmed);
            }
        }
    }
    return 0;
}