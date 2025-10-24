#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* to_jaden_case(const char* s) {
    if (s == NULL) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    int newWord = 1;
    for (size_t i = 0; i < n; i++) {
        unsigned char ch = (unsigned char)s[i];
        if (isspace(ch)) {
            out[i] = s[i];
            newWord = 1;
        } else {
            if (newWord && isalpha(ch)) {
                out[i] = (char)toupper(ch);
            } else if (isalpha(ch)) {
                out[i] = (char)tolower(ch);
            } else {
                out[i] = s[i];
            }
            newWord = 0;
        }
    }
    out[n] = '\0';
    return out;
}

int main(void) {
    const char* tests[] = {
        "How can mirrors be real if our eyes aren't real",
        "",
        "most trees are blue",
        "HELLO WORLD",
        "multiple   spaces\tand tabs"
    };
    size_t count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < count; i++) {
        char* res = to_jaden_case(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("\n");
        }
    }
    return 0;
}