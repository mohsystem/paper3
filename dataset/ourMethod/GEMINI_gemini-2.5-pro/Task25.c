#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration for the deallocation function
void free_numbered_lines(char** numbered_lines, size_t count);

/**
 * @brief Prepends line numbers to an array of strings.
 *
 * The numbering starts at 1. The format is "n: string".
 *
 * @param lines An array of constant C-strings.
 * @param count The number of strings in the `lines` array.
 * @return A new dynamically allocated array of strings.
 *         The caller is responsible for freeing the memory using free_numbered_lines.
 *         Returns NULL on failure.
 */
char** number(const char* const* lines, size_t count) {
    if (count == 0) {
        // Handle empty input: return a valid, heap-allocated 0-sized array.
        char** empty_result = malloc(0);
        return empty_result;
    }

    // Allocate memory for the array of pointers.
    char** result = malloc(count * sizeof(char*));
    if (result == NULL) {
        perror("Failed to allocate memory for result array");
        return NULL;
    }

    // Initialize pointers to NULL for safe cleanup in case of failure.
    for (size_t i = 0; i < count; ++i) {
        result[i] = NULL;
    }

    for (size_t i = 0; i < count; ++i) {
        // Calculate the length of the number string safely.
        int num_len = snprintf(NULL, 0, "%zu", i + 1);
        if (num_len < 0) {
            fprintf(stderr, "Error calculating number length\n");
            free_numbered_lines(result, count);
            return NULL;
        }

        // Calculate the length of the original string.
        size_t line_len = strlen(lines[i]);

        // Calculate total size: number + ": " + original_string + null_terminator
        // Check for potential size_t overflow.
        size_t required_size = (size_t)num_len + 2 + line_len + 1;
        if (required_size < (size_t)num_len || required_size < line_len) {
             fprintf(stderr, "Error: integer overflow in size calculation\n");
             free_numbered_lines(result, count);
             return NULL;
        }

        // Allocate memory for the new string.
        result[i] = malloc(required_size);
        if (result[i] == NULL) {
            perror("Failed to allocate memory for a new line");
            free_numbered_lines(result, count);
            return NULL;
        }

        // Safely format the new string, checking for errors.
        int written = snprintf(result[i], required_size, "%zu: %s", i + 1, lines[i]);
        if (written < 0 || (size_t)written >= required_size) {
            fprintf(stderr, "Error formatting the new line string\n");
            free_numbered_lines(result, count);
            return NULL;
        }
    }

    return result;
}

/**
 * @brief Frees the memory allocated by the `number` function.
 *
 * @param numbered_lines The array of strings to free.
 * @param count The number of elements in the array.
 */
void free_numbered_lines(char** numbered_lines, size_t count) {
    if (numbered_lines == NULL) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(numbered_lines[i]); // free each individual string
    }
    free(numbered_lines); // free the array of pointers
}

/**
 * @brief Helper function to print the result for testing.
 *
 * @param test_name A descriptive name for the test case.
 * @param arr The array of strings to print.
 * @param count The number of elements in the array.
 */
void print_string_array(const char* test_name, char** arr, size_t count) {
    printf("%s:\n", test_name);
    printf("[");
    if (arr != NULL) {
        for (size_t i = 0; i < count; ++i) {
            // arr[i] should not be NULL if number() succeeded
            if (arr[i] != NULL) {
                printf("\"%s\"", arr[i]);
            } else {
                printf("NULL");
            }
            if (i < count - 1) {
                printf(", ");
            }
        }
    }
    printf("]\n\n");
}

int main() {
    // Test Case 1: Empty list
    const char* test1_input[] = {};
    size_t test1_count = 0;
    char** test1_output = number(test1_input, test1_count);
    print_string_array("Test Case 1 (Empty)", test1_output, test1_count);
    free_numbered_lines(test1_output, test1_count);

    // Test Case 2: Simple list
    const char* test2_input[] = {"a", "b", "c"};
    size_t test2_count = sizeof(test2_input) / sizeof(test2_input[0]);
    char** test2_output = number(test2_input, test2_count);
    print_string_array("Test Case 2 (Simple)", test2_output, test2_count);
    free_numbered_lines(test2_output, test2_count);

    // Test Case 3: List with empty strings
    const char* test3_input[] = {"", "x", ""};
    size_t test3_count = sizeof(test3_input) / sizeof(test3_input[0]);
    char** test3_output = number(test3_input, test3_count);
    print_string_array("Test Case 3 (Empty Strings)", test3_output, test3_count);
    free_numbered_lines(test3_output, test3_count);

    // Test Case 4: List with more complex strings
    const char* test4_input[] = {"first line", "second line has spaces", "third;"};
    size_t test4_count = sizeof(test4_input) / sizeof(test4_input[0]);
    char** test4_output = number(test4_input, test4_count);
    print_string_array("Test Case 4 (Complex Strings)", test4_output, test4_count);
    free_numbered_lines(test4_output, test4_count);

    // Test Case 5: Longer list to test double-digit numbering
    const char* test5_input[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k"};
    size_t test5_count = sizeof(test5_input) / sizeof(test5_input[0]);
    char** test5_output = number(test5_input, test5_count);
    print_string_array("Test Case 5 (Double Digits)", test5_output, test5_count);
    free_numbered_lines(test5_output, test5_count);

    return 0;
}