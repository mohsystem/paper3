#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* reverse_string(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[i] = s[len - 1 - i];
    }
    out[len] = '\0';
    return out;
}

int main() {
    const char* tests[] = {"", "a", "abcde", "Hello, World!", "12345 racecar"};
    size_t n = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < n; ++i) {
        char* r = reverse_string(tests[i]);
        if (r) {
            printf("%s\n", r);
            free(r);
        } else {
            printf("(null)\n");
        }
    }
    return 0;
}