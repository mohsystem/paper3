#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Concatenates an array of strings into a single new string.
 * @note The caller is responsible for freeing the memory of the returned string using free().
 *
 * @param strings An array of C-style strings (char*).
 * @param count The number of strings in the array.
 * @return A pointer to a new dynamically allocated string containing the concatenation, or NULL on failure.
 */
char* concatenateStrings(const char* strings[], int count) {
    size_t total_length = 0;
    for (int i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            total_length += strlen(strings[i]);
        }
    }

    // Allocate memory for the concatenated string (+1 for null terminator)
    char* result = (char*)malloc(total_length + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Use a pointer to copy strings efficiently
    char* current_pos = result;
    for (int i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            size_t len = strlen(strings[i]);
            memcpy(current_pos, strings[i], len);
            current_pos += len;
        }
    }
    
    *current_pos = '\0'; // Add the final null terminator

    return result;
}

void run_test_case(const char* name, const char* input_str, const char* strings[], int count) {
    printf("%s Input: %s\n", name, input_str);
    char* concatenated = concatenateStrings(strings, count);
    if (concatenated) {
        printf("Output: %s\n", concatenated);
        free(concatenated); // Free the allocated memory
    }
    printf("\n");
}

int main() {
    // Test Case 1
    const char* test1[] = {"Hello", " ", "World", "!"};
    run_test_case("Test Case 1", "{\"Hello\", \" \", \"World\", \"!\"}", test1, 4);

    // Test Case 2
    const char* test2[] = {"Java", ", ", "Python", ", ", "C++", ", ", "C"};
    run_test_case("Test Case 2", "{\"Java\", \", \", \"Python\", \", \", \"C++\", \", \", \"C\"}", test2, 7);

    // Test Case 3
    const char* test3[] = {"One"};
    run_test_case("Test Case 3", "{\"One\"}", test3, 1);
    
    // Test Case 4: With empty strings
    const char* test4[] = {"", "leading", " and ", "trailing", ""};
    run_test_case("Test Case 4", "{\\\"\\\", \\\"leading\\\", \\\" and \\\", \\\"trailing\\\", \\\"\\\"}", test4, 5);

    // Test Case 5: Empty input array
    const char* test5[] = {};
    run_test_case("Test Case 5", "{}", test5, 0);

    return 0;
}