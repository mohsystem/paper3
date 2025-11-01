#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

char* copy_and_print(const char* input) {
    if (input == NULL) {
        input = "";
    }

    size_t len = strlen(input);
    if (len > SIZE_MAX - 1) {
        return NULL; // overflow guard
    }

    char* src = (char*)malloc(len + 1);
    if (src == NULL) {
        return NULL;
    }
    memcpy(src, input, len + 1); // includes null terminator

    char* dst = (char*)malloc(len + 1);
    if (dst == NULL) {
        free(src);
        return NULL;
    }
    memcpy(dst, src, len + 1);

    printf("%s\n", dst);

    free(src);
    return dst; // caller must free
}

int main(void) {
    const char* tests[5] = {
        "",
        "Hello, World!",
        "Secure Copy 123!@#",
        "こんにちは",
        NULL
    };

    // Prepare long string for the 5th test
    char longStr[101];
    for (int i = 0; i < 100; i++) longStr[i] = 'a';
    longStr[100] = '\0';
    tests[4] = longStr;

    for (int i = 0; i < 5; i++) {
        char* out = copy_and_print(tests[i]);
        if (out != NULL) {
            free(out);
        }
    }
    return 0;
}