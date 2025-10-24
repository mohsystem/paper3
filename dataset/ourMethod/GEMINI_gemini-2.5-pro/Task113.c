#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Concatenates an array of C-style strings.
 *
 * This function safely concatenates an array of strings by first calculating
 * the total required memory, allocating it, and then copying the strings
 * into the new buffer.
 *
 * @param strings An array of C-style strings (char*). NULL pointers in the array are ignored.
 * @param count The number of strings in the array.
 * @return A pointer to a newly allocated string containing the concatenation.
 *         The caller is responsible for freeing this memory using free().
 *         Returns NULL if memory allocation fails.
 *         Returns a pointer to a dynamically allocated empty string ("") if count is 0 or strings is NULL.
 */
char* concatenateStrings(const char* const strings[], int count) {
    if (strings == NULL || count <= 0) {
        // Return a dynamically allocated empty string for API consistency.
        // The caller can always safely call free() on the result.
        char* empty_str = (char*)malloc(1);
        if (empty_str) {
            empty_str[0] = '\0';
        }
        return empty_str;
    }

    size_t total_length = 0;
    for (int i = 0; i < count; ++i) {
        if (strings[i] != NULL) {
            total_length += strlen(strings[i]);
        }
    }

    // Allocate memory for the concatenated string + null terminator.
    char* result = (char*)malloc(total_length + 1);
    if (result == NULL) {
        return NULL; // Propagate memory allocation failure.
    }

    // Use a pointer to track the current position in the result buffer.
    char* current_pos = result;
    for (int i = 0; i < count; ++i) {
        if (strings[i] != NULL) {
            size_t len = strlen(strings[i]);
            // memcpy is safe here because we've calculated and allocated enough space.
            memcpy(current_pos, strings[i], len);
            current_pos += len;
        }
    }

    // Add the null terminator at the very end.
    *current_pos = '\0';

    return result;
}

void run_test(const char* test_name, const char* const strings[], int count) {
    char* result = concatenateStrings(strings, count);
    if (result) {
        printf("%s: %s\n", test_name, result);
        free(result); // IMPORTANT: Free the allocated memory.
    } else {
        printf("%s: Memory allocation failed.\n", test_name);
    }
}

int main() {
    // Test Case 1: Basic concatenation
    const char* test1[] = {"Hello", ", ", "World", "!"};
    run_test("Test Case 1", test1, sizeof(test1) / sizeof(test1[0]));

    // Test Case 2: Array with NULL and empty strings
    const char* test2[] = {"C", " ", NULL, "is", "", " powerful."};
    run_test("Test Case 2", test2, sizeof(test2) / sizeof(test2[0]));

    // Test Case 3: Empty array (by passing count = 0)
    const char* test3[] = {"This should not be printed"};
    run_test("Test Case 3", test3, 0);

    // Test Case 4: Null input array pointer
    run_test("Test Case 4", NULL, 5);

    // Test Case 5: Single string
    const char* test5[] = {"SingleString"};
    run_test("Test Case 5", test5, sizeof(test5) / sizeof(test5[0]));

    return 0;
}