#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 1000000

char* reverse_string(const char* s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = strnlen(s, MAX_LEN + 1);
    if (len > MAX_LEN) {
        return NULL;
    }
    char* out = (char*)malloc(len + 1);
    if (out == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        out[i] = s[len - 1 - i];
    }
    out[len] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "",
        "a",
        "abc",
        "Hello, World!",
        "12345!@#$%"
    };
    for (int i = 0; i < 5; ++i) {
        char* r = reverse_string(tests[i]);
        if (r == NULL) {
            puts("ERROR");
        } else {
            puts(r);
            free(r);
        }
    }
    return 0;
}