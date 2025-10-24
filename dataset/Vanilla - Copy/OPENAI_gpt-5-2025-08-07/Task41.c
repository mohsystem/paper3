#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* process(const char* s) {
    if (s == NULL) {
        char* r = (char*)malloc(1);
        if (r) r[0] = '\0';
        return r;
    }
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    if (start == len) {
        char* r = (char*)malloc(1);
        if (r) r[0] = '\0';
        return r;
    }
    size_t end = len - 1;
    while (end > start && isspace((unsigned char)s[end])) end--;

    size_t maxNeeded = end - start + 1;
    char* out = (char*)malloc(maxNeeded + 1);
    if (!out) return NULL;

    size_t w = 0;
    int prevSpace = 0;
    for (size_t i = start; i <= end; i++) {
        unsigned char ch = (unsigned char)s[i];
        if (isspace(ch)) {
            if (!prevSpace) {
                out[w++] = ' ';
                prevSpace = 1;
            }
        } else {
            out[w++] = (char)toupper(ch);
            prevSpace = 0;
        }
    }
    out[w] = '\0';
    return out;
}

int main(void) {
    char buffer[2048];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        size_t l = strlen(buffer);
        while (l > 0 && (buffer[l - 1] == '\n' || buffer[l - 1] == '\r')) {
            buffer[--l] = '\0';
        }
        char* result = process(buffer);
        if (result) {
            puts(result);
            free(result);
        }
    }

    const char* tests[5] = {
        "  hello   world  ",
        "\tMixed\tCase Input\n",
        "already CLEAN",
        "   ",
        "Symbols 123 !@#   test"
    };
    for (int i = 0; i < 5; i++) {
        char* r = process(tests[i]);
        if (r) {
            puts(r);
            free(r);
        }
    }
    return 0;
}