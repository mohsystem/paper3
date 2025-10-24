#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* toJadenCase(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    int newWord = 1;
    for (size_t i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)s[i];
        if (newWord && isalpha(ch)) {
            out[i] = (char)toupper(ch);
        } else if (isalpha(ch)) {
            out[i] = (char)tolower(ch);
        } else {
            out[i] = (char)ch;
        }
        newWord = isspace(ch) || ch == '-';
    }
    out[len] = '\0';
    return out;
}

int main() {
    const char* tests[5] = {
        "How can mirrors be real if our eyes aren't real",
        "",
        "not jaden-cased string",
        "this IS a tESt",
        NULL
    };
    for (int i = 0; i < 5; i++) {
        const char* t = tests[i];
        char* res = toJadenCase(t);
        printf("Input:  %s\n", t ? t : "NULL");
        if (res) {
            printf("Output: %s\n", res);
            free(res);
        } else {
            printf("Output: NULL\n");
        }
        printf("---\n");
    }
    return 0;
}