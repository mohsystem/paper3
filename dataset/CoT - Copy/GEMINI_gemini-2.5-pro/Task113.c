#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Concatenates an array of strings into a single, dynamically allocated string.
 *
 * This function securely concatenates strings by first calculating the exact
 * required buffer size, allocating memory, and then copying the data. This
 * prevents buffer overflows. It also handles potential NULL pointers in the
 * input array.
 *
 * @param strings An array of C-style strings (char*).
 * @param count The number of strings in the array.
 * @return A pointer to a new dynamically allocated string containing the result.
 *         The caller is responsible for freeing this memory with free().
 *         Returns NULL if memory allocation fails.
 */
char* concatenateStrings(char* strings[], int count) {
    if (count <= 0 || strings == NULL) {
        // Return an empty, allocated string so the caller can safely free it.
        char* empty_str = (char*)malloc(1);
        if (empty_str != NULL) {
            empty_str[0] = '\0';
        }
        return empty_str;
    }

    // Step 1: Calculate the total length required for the new string.
    size_t total_length = 0;
    for (int i = 0; i < count; ++i) {
        if (strings[i] != NULL) { // Defensively handle NULL pointers
            total_length += strlen(strings[i]);
        }
    }

    // Step 2: Allocate memory (+1 for the null terminator).
    char* result = (char*)malloc(total_length + 1);
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }
    
    // Step 3: Initialize the result string to be empty for the first strcat.
    result[0] = '\0';

    // Step 4: Copy the strings into the result buffer.
    for (int i = 0; i < count; ++i) {
         if (strings[i] != NULL) {
            strcat(result, strings[i]);
         }
    }

    return result;
}

void runTestCase(const char* testName, char* testData[], int count) {
    printf("%s: [", testName);
    for (int i = 0; i < count; ++i) {
        // Handle printing of potential NULLs for clarity
        if (testData[i] != NULL) {
            printf("\"%s\"", testData[i]);
        } else {
            printf("NULL");
        }
        if (i < count - 1) {
            printf(", ");
        }
    }
    printf("]\n");

    char* concatenated = concatenateStrings(testData, count);
    if (concatenated != NULL) {
        printf("Result: %s\n\n", concatenated);
        // CRITICAL: Free the dynamically allocated memory after use.
        free(concatenated);
    } else {
        printf("Result: FAILED (NULL returned)\n\n");
    }
}

int main() {
    // Test Case 1: Basic concatenation
    char* test1[] = {"Hello", " ", "World", "!"};
    runTestCase("Test Case 1", test1, 4);

    // Test Case 2: Another basic case
    char* test2[] = {"C", " ", "is", " ", "foundational."};
    runTestCase("Test Case 2", test2, 5);

    // Test Case 3: Empty array (by passing count 0)
    char* test3[] = {};
    runTestCase("Test Case 3", test3, 0);

    // Test Case 4: Array with a single element
    char* test4[] = {"SingleString"};
    runTestCase("Test Case 4", test4, 1);

    // Test Case 5: Array with empty strings and a NULL pointer
    char* test5[] = {"part1", "", "part2", NULL, "part3"};
    runTestCase("Test Case 5", test5, 5);

    return 0;
}