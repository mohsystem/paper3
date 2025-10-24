#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Reverses a null-terminated string.
 * This function allocates new memory for the reversed string.
 * The caller is responsible for freeing this memory using free().
 *
 * @param str The constant C-string to be reversed.
 * @return A pointer to the newly allocated reversed string,
 *         or NULL if the input is NULL or if memory allocation fails.
 */
char* reverseString(const char* str) {
    // 1. Validate input: Check for NULL pointer to prevent crashes.
    if (str == NULL) {
        return NULL;
    }

    // 2. Get the length of the string.
    size_t len = strlen(str);

    // 3. Allocate memory for the new string (length + 1 for the null terminator).
    char* reversed_str = (char*)malloc(len + 1);

    // 4. Check if memory allocation was successful.
    if (reversed_str == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    // 5. Copy characters from the end of the source to the start of the destination.
    for (size_t i = 0; i < len; i++) {
        reversed_str[i] = str[len - 1 - i];
    }

    // 6. Add the null terminator to make it a valid C-string.
    reversed_str[len] = '\0';

    return reversed_str;
}

int main() {
    // 5 test cases
    const char* testCases[] = {
        "hello world",
        "12345",
        "a",
        "",
        "Palindrome"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        char* reversed = reverseString(testCases[i]);
        // Always check if the returned pointer from a function that allocates memory is not NULL.
        if (reversed != NULL) {
            printf("Original: \"%s\", Reversed: \"%s\"\n", testCases[i], reversed);
            // Free the memory allocated by reverseString to prevent memory leaks.
            free(reversed);
        }
    }

    return 0;
}