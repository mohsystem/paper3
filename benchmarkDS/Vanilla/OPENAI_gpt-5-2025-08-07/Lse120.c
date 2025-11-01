#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* copy_and_print(const char* input) {
    size_t len = strlen(input);
    char* chunk1 = (char*)malloc(len + 1);
    char* chunk2 = (char*)malloc(len + 1);
    if (!chunk1 || !chunk2) {
        free(chunk1);
        free(chunk2);
        return NULL;
    }
    memcpy(chunk1, input, len + 1);
    memcpy(chunk2, chunk1, len + 1);
    printf("%s\n", chunk2);
    free(chunk1);
    return chunk2;
}

int main() {
    const char* tests[5] = {
        "Hello",
        "",
        "Memory test 123",
        "こんにちは",
        "Emoji 😀🚀"
    };
    for (int i = 0; i < 5; ++i) {
        char* r = copy_and_print(tests[i]);
        if (r) {
            printf("Returned: %s\n", r);
            free(r);
        }
    }
    return 0;
}