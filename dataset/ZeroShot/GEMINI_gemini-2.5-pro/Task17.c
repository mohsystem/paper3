#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

char* toJadenCase(const char* phrase) {
    if (phrase == NULL || phrase[0] == '\0') {
        return NULL; // Per instructions, return NULL for null or empty input
    }
    
    size_t len = strlen(phrase);
    // Allocate memory for the new string (+1 for the null terminator)
    // Using calloc to initialize memory to zero is a good practice.
    char* result = (char*)calloc(len + 1, sizeof(char));
    if (result == NULL) {
        // Handle memory allocation failure
        perror("Failed to allocate memory");
        return NULL;
    }

    // Copy the original string to the new buffer
    // strncpy is safer than strcpy as it prevents buffer overflows
    strncpy(result, phrase, len);
    result[len] = '\0'; // Ensure null termination

    bool capitalize = true;
    for (size_t i = 0; i < len; ++i) {
        if (isspace((unsigned char)result[i])) {
            capitalize = true;
        } else if (capitalize && isalpha((unsigned char)result[i])) {
            result[i] = toupper((unsigned char)result[i]);
            capitalize = false;
        }
    }

    return result;
}

int main() {
    // Test Case 1
    const char* test1 = "How can mirrors be real if our eyes aren't real";
    char* result1 = toJadenCase(test1);
    printf("Original: %s\n", test1);
    if (result1) {
        printf("Jaden-Cased: %s\n", result1);
        free(result1); // Free the allocated memory
    }

    // Test Case 2
    const char* test2 = "most trees are blue";
    char* result2 = toJadenCase(test2);
    printf("\nOriginal: %s\n", test2);
    if (result2) {
        printf("Jaden-Cased: %s\n", result2);
        free(result2);
    }
    
    // Test Case 3
    const char* test3 = "";
    char* result3 = toJadenCase(test3);
    printf("\nOriginal: \"%s\"\n", test3);
    printf("Jaden-Cased: %s\n", result3 == NULL ? "(null)" : result3);
    // No need to free if result is NULL
    
    // Test Case 4
    const char* test4 = "a b c d";
    char* result4 = toJadenCase(test4);
    printf("\nOriginal: %s\n", test4);
    if (result4) {
        printf("Jaden-Cased: %s\n", result4);
        free(result4);
    }

    // Test Case 5
    const char* test5 = "if a book store never runs out of a certain book, does that mean that nobody reads it, or everybody reads it";
    char* result5 = toJadenCase(test5);
    printf("\nOriginal: %s\n", test5);
    if (result5) {
        printf("Jaden-Cased: %s\n", result5);
        free(result5);
    }

    return 0;
}