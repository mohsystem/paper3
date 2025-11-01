#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

char* process(const char* input) {
    if (input == NULL) {
        input = "";
    }

    size_t len = strlen(input);

    if (len > SIZE_MAX - 1) {
        return NULL;
    }

    char* chunk1 = (char*)malloc(len + 1);
    if (chunk1 == NULL) {
        return NULL;
    }
    memcpy(chunk1, input, len);
    chunk1[len] = '\0';

    char* chunk2 = (char*)malloc(len + 1);
    if (chunk2 == NULL) {
        free(chunk1);
        return NULL;
    }
    memcpy(chunk2, chunk1, len);
    chunk2[len] = '\0';

    printf("%s\n", chunk2);

    free(chunk1);
    chunk1 = NULL;

    printf("%s\n", chunk2);

    if (len > (SIZE_MAX - 3) / 2) {
        free(chunk2);
        return NULL;
    }
    size_t outSize = 2 * len + 3;
    char* out = (char*)malloc(outSize);
    if (out == NULL) {
        free(chunk2);
        return NULL;
    }
    memcpy(out, chunk2, len);
    out[len] = '\n';
    memcpy(out + len + 1, chunk2, len);
    out[2 * len + 1] = '\n';
    out[2 * len + 2] = '\0';

    free(chunk2);
    chunk2 = NULL;

    return out;
}

int main(void) {
    const char* tests[5] = {
        "Hello, World!",
        "",
        "UTF-8: こんにちは",
        "Numbers: 1234567890",
        "Line1\nLine2\tTabbed!"
    };

    for (int i = 0; i < 5; ++i) {
        char* out = process(tests[i]);
        printf("Returned:\n");
        if (out != NULL) {
            printf("%s", out);
            free(out);
        } else {
            printf("(null)\n");
        }
    }
    return 0;
}