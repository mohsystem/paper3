#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* copy_and_manage(const char* input) {
    if (input == NULL) input = "";
    size_t n = strlen(input) + 1; // include null terminator
    char* first = (char*)malloc(n);
    if (!first) return NULL;
    memcpy(first, input, n);

    char* second = (char*)malloc(n);
    if (!second) {
        free(first);
        return NULL;
    }
    memcpy(second, first, n);

    // Free first chunk
    free(first);

    return second; // caller prints and frees
}

int main(void) {
    const char* tests[5] = {"Hello", "World!", "Memory Test", "12345", "Symbols!@#"};
    for (int i = 0; i < 5; ++i) {
        char* second = copy_and_manage(tests[i]);
        if (!second) {
            fprintf(stderr, "Allocation failed\n");
            continue;
        }
        printf("%s\n", second);
        printf("%s\n", second);
        // Free second chunk
        free(second);
    }
    return 0;
}