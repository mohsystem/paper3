#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* encrypt(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (c >= 'A' && c <= 'Z') {
            out[i] = (char)('A' + (c - 'A' + 3) % 26);
        } else if (c >= 'a' && c <= 'z') {
            out[i] = (char)('a' + (c - 'a' + 3) % 26);
        } else if (c >= '0' && c <= '9') {
            out[i] = (char)('0' + (c - '0' + 5) % 10);
        } else {
            out[i] = c;
        }
    }
    out[n] = '\0';
    return out;
}

int main() {
    const char* tests[5] = {
        "Hello, World!",
        "xyz XYZ",
        "Encrypt123",
        "",
        "Attack at dawn! 09"
    };
    for (int i = 0; i < 5; ++i) {
        char* enc = encrypt(tests[i]);
        if (enc) {
            printf("Input:  %s\n", tests[i]);
            printf("Output: %s\n\n", enc);
            free(enc);
        } else {
            printf("Memory allocation failed\n");
        }
    }
    return 0;
}