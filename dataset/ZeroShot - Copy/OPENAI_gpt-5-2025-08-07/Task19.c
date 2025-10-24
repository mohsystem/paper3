#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* spinWords(const char* s) {
    if (s == NULL) {
        char* out = (char*)malloc(1);
        if (out != NULL) {
            out[0] = '\0';
        }
        return out;
    }
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (out == NULL) {
        return NULL;
    }
    size_t i = 0;
    size_t o = 0;
    while (i < n) {
        if (s[i] == ' ') {
            out[o++] = ' ';
            i++;
        } else {
            size_t start = i;
            while (i < n && s[i] != ' ') {
                i++;
            }
            size_t len = i - start;
            if (len >= 5) {
                for (size_t k = 0; k < len; ++k) {
                    out[o + k] = s[i - 1 - k];
                }
            } else {
                for (size_t k = 0; k < len; ++k) {
                    out[o + k] = s[start + k];
                }
            }
            o += len;
        }
    }
    out[o] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "",
        "Hi   there"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = spinWords(tests[i]);
        if (out == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        printf("Input: \"%s\" -> Output: \"%s\"\n", tests[i], out);
        free(out);
    }
    return 0;
}