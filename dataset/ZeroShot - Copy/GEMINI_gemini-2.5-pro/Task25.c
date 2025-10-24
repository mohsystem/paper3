#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Prepends line numbers to an array of strings.
 * The caller is responsible for freeing the returned array and its strings
 * using the free_numbered_lines function.
 * 
 * @param lines An array of constant C-style strings.
 * @param n The number of strings in the array.
 * @return A new dynamically allocated array of C-style strings with line numbers,
 *         or NULL if n is 0 or if a memory allocation error occurs.
 */
char** number(const char* const lines[], size_t n) {
    if (n == 0) {
        return NULL;
    }

    char** numbered_lines = (char**)malloc(n * sizeof(char*));
    if (!numbered_lines) {
        perror("Failed to allocate memory for lines array");
        return NULL;
    }
    
    for (size_t i = 0; i < n; ++i) {
        // Use snprintf with NULL buffer to securely determine the required size.
        // Size needed = length of number + ": " + string content + null terminator.
        int len = snprintf(NULL, 0, "%zu: %s", i + 1, lines[i]);
        if (len < 0) {
             fprintf(stderr, "Encoding error in snprintf\n");
             goto cleanup;
        }

        // Allocate memory for the new string (+1 for null terminator).
        numbered_lines[i] = (char*)malloc(len + 1);
        if (!numbered_lines[i]) {
            perror("Failed to allocate memory for a line");
            goto cleanup;
        }

        // Format the string into the allocated buffer.
        sprintf(numbered_lines[i], "%zu: %s", i + 1, lines[i]);
    }
    
    return numbered_lines;

cleanup:
    // Error handling: free any memory that was successfully allocated before the error.
    for (size_t j = 0; j < n && numbered_lines[j] != NULL; ++j) {
        free(numbered_lines[j]);
    }
    free(numbered_lines);
    return NULL;
}

/**
 * Helper function to free the memory allocated by the 'number' function.
 */
void free_numbered_lines(char** lines, size_t n) {
    if (!lines) {
        return;
    }
    for (size_t i = 0; i < n; ++i) {
        free(lines[i]); // Free each individual string
    }
    free(lines); // Free the array of pointers
}

/**
 * Helper function to print an array of strings for testing.
 */
void print_string_array(const char* label, const char* const arr[], size_t n) {
    printf("%s[", label);
    if (arr != NULL) {
        for (size_t i = 0; i < n; ++i) {
            printf("\"%s\"", arr[i]);
            if (i < n - 1) {
                printf(", ");
            }
        }
    }
    printf("]\n");
}

int main() {
    // Test Case 1: Empty list
    const char* test1[] = {};
    size_t n1 = sizeof(test1) / sizeof(test1[0]);
    print_string_array("Test Case 1 Input: ", test1, n1);
    char** result1 = number(test1, n1);
    print_string_array("Output: ", (const char* const*)result1, n1);
    free_numbered_lines(result1, n1);
    printf("\n");

    // Test Case 2: Simple list
    const char* test2[] = {"a", "b", "c"};
    size_t n2 = sizeof(test2) / sizeof(test2[0]);
    print_string_array("Test Case 2 Input: ", test2, n2);
    char** result2 = number(test2, n2);
    if (result2) {
        print_string_array("Output: ", (const char* const*)result2, n2);
        free_numbered_lines(result2, n2);
    }
    printf("\n");

    // Test Case 3: List with an empty string
    const char* test3[] = {"", "hello", ""};
    size_t n3 = sizeof(test3) / sizeof(test3[0]);
    print_string_array("Test Case 3 Input: ", test3, n3);
    char** result3 = number(test3, n3);
    if (result3) {
        print_string_array("Output: ", (const char* const*)result3, n3);
        free_numbered_lines(result3, n3);
    }
    printf("\n");
    
    // Test Case 4: List with longer strings
    const char* test4[] = {"First line", "Second line", "Third line"};
    size_t n4 = sizeof(test4) / sizeof(test4[0]);
    print_string_array("Test Case 4 Input: ", test4, n4);
    char** result4 = number(test4, n4);
    if (result4) {
        print_string_array("Output: ", (const char* const*)result4, n4);
        free_numbered_lines(result4, n4);
    }
    printf("\n");
    
    // Test Case 5: List with one element
    const char* test5[] = {"single"};
    size_t n5 = sizeof(test5) / sizeof(test5[0]);
    print_string_array("Test Case 5 Input: ", test5, n5);
    char** result5 = number(test5, n5);
    if (result5) {
        print_string_array("Output: ", (const char* const*)result5, n5);
        free_numbered_lines(result5, n5);
    }
    printf("\n");

    return 0;
}