#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Reverses a given C-style string.
 * @param str The null-terminated string to be reversed.
 * @return A new dynamically allocated string which is the reverse of the input.
 *         The caller is responsible for freeing this memory.
 */
char* reverseString(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    size_t len = strlen(str);
    char* reversed_str = (char*)malloc(len + 1);

    if (reversed_str == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    for (size_t i = 0; i < len; i++) {
        reversed_str[i] = str[len - 1 - i];
    }
    reversed_str[len] = '\0'; // Null-terminate the new string

    return reversed_str;
}

int main() {
    const char* testCases[] = {
        "hello",
        "world",
        "C",
        "12345",
        ""
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("C Test Cases:\n");
    for (int i = 0; i < numTestCases; i++) {
        char* reversed = reverseString(testCases[i]);
        if (reversed != NULL) {
            printf("Original: \"%s\", Reversed: \"%s\"\n", testCases[i], reversed);
            free(reversed); // Free the allocated memory
        }
    }

    return 0;
}