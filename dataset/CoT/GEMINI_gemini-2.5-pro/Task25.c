#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Prepends line numbers to an array of strings.
 *
 * NOTE: The returned array and its string elements are dynamically allocated.
 * The caller is responsible for freeing this memory using the `free_numbered_lines` function.
 *
 * @param lines The array of C strings.
 * @param count The number of strings in the array.
 * @return A new dynamically allocated array of strings with line numbers, or NULL on failure.
 */
char** number(const char** lines, size_t count) {
    if (count == 0) {
        // Return a dynamically allocated array of size 0, which is valid to free.
        return (char**)malloc(0); 
    }
    
    // Allocate memory for the array of pointers
    char** result = (char**)malloc(count * sizeof(char*));
    if (result == NULL) {
        return NULL; // Memory allocation failed for the main array
    }

    for (size_t i = 0; i < count; ++i) {
        // Calculate the required buffer size for the new string "i+1: line\0"
        // snprintf with a NULL buffer returns the number of characters that would have been written.
        // The line number starts from 1.
        int num_len = snprintf(NULL, 0, "%zu", i + 1);
        size_t line_len = strlen(lines[i]);
        // size = (number) + (": ") + (string) + ('\0')
        size_t total_len = num_len + 2 + line_len + 1;

        result[i] = (char*)malloc(total_len * sizeof(char));
        if (result[i] == NULL) {
            // Cleanup previously allocated memory on failure
            for (size_t j = 0; j < i; ++j) {
                free(result[j]);
            }
            free(result);
            return NULL;
        }

        // Safely format the string into the allocated buffer
        snprintf(result[i], total_len, "%zu: %s", i + 1, lines[i]);
    }

    return result;
}

/**
 * Frees the memory allocated by the `number` function.
 *
 * @param numbered_lines The array of strings to free.
 * @param count The number of strings in the array.
 */
void free_numbered_lines(char** numbered_lines, size_t count) {
    if (numbered_lines == NULL) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(numbered_lines[i]);
    }
    free(numbered_lines);
}

// Helper function to run a single test case
void run_test(const char* test_name, const char** input, size_t count) {
    printf("%s (Input: [", test_name);
    for (size_t i = 0; i < count; ++i) {
        printf("\"%s\"", input[i]);
        if (i < count - 1) printf(", ");
    }
    printf("])\n");

    char** output = number(input, count);
    
    if (output != NULL) {
        printf("Output: [");
        for (size_t i = 0; i < count; ++i) {
            printf("\"%s\"", output[i]);
            if (i < count - 1) printf(", ");
        }
        printf("]\n\n");
        free_numbered_lines(output, count);
    } else {
        printf("Output: (Allocation failed or empty)\n\n");
        // If count was 0, output is valid but has 0 elements. Free it.
        if (count == 0 && output) free(output);
    }
}

int main() {
    // Test Case 1: Empty list
    const char* test1[] = {};
    run_test("Test Case 1", test1, 0);

    // Test Case 2: Standard list
    const char* test2[] = {"a", "b", "c"};
    run_test("Test Case 2", test2, 3);

    // Test Case 3: List with empty strings
    const char* test3[] = {"", "", "d"};
    run_test("Test Case 3", test3, 3);

    // Test Case 4: List with two strings
    const char* test4[] = {"Hello", "World"};
    run_test("Test Case 4", test4, 2);

    // Test Case 5: List with one string
    const char* test5[] = {"single line"};
    run_test("Test Case 5", test5, 1);

    return 0;
}