#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Reverses a given C-style string by creating a new reversed string.
 * 
 * This function is designed to be secure. It prevents buffer overflows by
 * dynamically allocating exactly the required amount of memory. It checks for
 * memory allocation failure and handles NULL input gracefully. It does not
 * modify the original string.
 * 
 * @param str The null-terminated string to be reversed.
 * @return A new dynamically allocated string which is the reverse of the input.
 *         The caller is responsible for freeing this memory using free().
 *         Returns NULL if the input string is NULL or if memory allocation fails.
 */
char* reverseString(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    size_t len = strlen(str);
    // Allocate memory for the reversed string (+1 for the null terminator '\0')
    char* reversed_str = (char*)malloc(len + 1);

    // Security: Check if malloc was successful to prevent null pointer dereference.
    if (reversed_str == NULL) {
        perror("Error: Failed to allocate memory in reverseString");
        return NULL;
    }

    // Copy characters from the end of the source to the beginning of the destination
    for (size_t i = 0; i < len; i++) {
        reversed_str[i] = str[len - 1 - i];
    }
    
    // Add the null terminator to make it a valid C-string.
    reversed_str[len] = '\0';

    return reversed_str;
}

int main() {
    const char* testCases[] = {
        "hello",
        "C Language",
        "12345",
        "",
        "a single char"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C String Reversal ---\n");
    for (int i = 0; i < numTestCases; i++) {
        const char* original = testCases[i];
        char* reversed = reverseString(original);
        
        if (reversed != NULL) {
            printf("Original: \"%s\", Reversed: \"%s\"\n", original, reversed);
            // Security: Free the dynamically allocated memory to prevent memory leaks.
            free(reversed);
        } else {
            fprintf(stderr, "Failed to reverse string: \"%s\"\n", original);
        }
    }
    
    return 0;
}