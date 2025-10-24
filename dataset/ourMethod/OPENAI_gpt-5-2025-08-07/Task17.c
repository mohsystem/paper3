#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

char* to_jaden_case(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    size_t len = strlen(input);
    char* out = (char*)malloc(len + 1);
    if (out == NULL) {
        return NULL;
    }

    bool at_word_start = true;
    for (size_t i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)input[i];
        if (isspace(ch)) {
            out[i] = input[i];
            at_word_start = true;
        } else {
            if (at_word_start && isalpha(ch)) {
                out[i] = (char)toupper(ch);
            } else {
                out[i] = input[i];
            }
            at_word_start = false;
        }
    }
    out[len] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "How can mirrors be real if our eyes aren't real",
        "",
        "hello\tworld\nfrom  c",
        "already Capitalized Words",
        NULL
    };

    for (int i = 0; i < 5; ++i) {
        char* result = to_jaden_case(tests[i]);
        if (result == NULL) {
            printf("Input:  [NULL]\n");
            printf("Output: [NULL]\n");
        } else {
            printf("Input:  [%s]\n", tests[i]);
            printf("Output: [%s]\n", result);
            free(result);
        }
        printf("----\n");
    }
    return 0;
}