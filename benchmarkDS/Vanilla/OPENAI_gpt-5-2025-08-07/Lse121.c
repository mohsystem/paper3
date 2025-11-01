#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* simulate_memory(const char* input) {
    size_t n = strlen(input);

    char* chunk1 = (char*)malloc(n + 1);
    char* chunk2 = (char*)malloc(n + 1);
    if (!chunk1 || !chunk2) {
        if (chunk1) free(chunk1);
        if (chunk2) free(chunk2);
        return NULL;
    }

    memcpy(chunk1, input, n + 1);
    memcpy(chunk2, chunk1, n + 1);

    printf("%s\n", chunk2);

    free(chunk1);

    printf("%s\n", chunk2);

    char* result = (char*)malloc(2 * n + 2);
    if (!result) {
        free(chunk2);
        return NULL;
    }
    memcpy(result, input, n);
    result[n] = '\n';
    memcpy(result + n + 1, input, n);
    result[2 * n + 1] = '\0';

    free(chunk2);

    return result;
}

int main() {
    const char* tests[5] = {
        "Test1",
        "Second",
        "A longer string for memory copy",
        "1234567890",
        "EdgeCase!"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = simulate_memory(tests[i]);
        if (res) free(res);
    }
    return 0;
}