#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* spin_words(const char* s) {
    size_t n = strlen(s);
    char* res = (char*)malloc(n + 1);
    if (!res) return NULL;

    const char* p = s;
    char* q = res;

    while (*p) {
        if (*p == ' ') {
            *q++ = *p++;
        } else {
            const char* start = p;
            int len = 0;
            while (*p && *p != ' ') { ++p; ++len; }
            if (len >= 5) {
                const char* t = p - 1;
                for (int i = 0; i < len; ++i) *q++ = *t--;
            } else {
                for (int i = 0; i < len; ++i) *q++ = start[i];
            }
        }
    }
    *q = '\0';
    return res;
}

int main() {
    const char* tests[5] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "CodeWars is nice place"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = spin_words(tests[i]);
        if (out) {
            printf("%s\n", out);
            free(out);
        }
    }
    return 0;
}