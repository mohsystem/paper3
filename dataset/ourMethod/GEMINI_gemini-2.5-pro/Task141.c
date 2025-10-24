#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reverses a given string by creating a new reversed string on the heap.
 *
 * @param str The null-terminated string to reverse.
 * @return A pointer to a new heap-allocated reversed string.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL if the input is NULL or if memory allocation fails.
 */
char* reverseString(const char* str) {
    // Rule #4: Validate input. Handle NULL pointer to prevent crashes.
    if (str == NULL) {
        return NULL;
    }

    size_t len = strlen(str);
    
    // Rule #1 & #2: Allocate buffer of the correct size, including space for the null terminator.
    char* reversed_str = (char*)malloc(len + 1);

    // Check if memory allocation was successful.
    if (reversed_str == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Copy characters from the end of the source to the beginning of the destination.
    for (size_t i = 0; i < len; i++) {
        reversed_str[i] = str[len - 1 - i];
    }
    
    // Crucial: Add the null terminator to the end of the new string.
    reversed_str[len] = '\0';

    return reversed_str;
}

int main(void) {
    const char* testCases[] = {
        "Hello, World!",
        "madam",
        "12345",
        "",
        "a"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        printf("Original: \"%s\"\n", testCases[i]);
        char* reversed = reverseString(testCases[i]);
        if (reversed != NULL) {
            printf("Reversed: \"%s\"\n", reversed);
            free(reversed); // Free the heap-allocated memory.
        } else {
            printf("Reversed: (null)\n");
        }
        printf("\n");
    }

    return 0;
}