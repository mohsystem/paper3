#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Securely concatenates multiple C-strings. This function prevents buffer overflows
 * by first calculating the exact required buffer size, allocating it, and then
 * copying the data. It is more efficient than repeated strcat calls.
 *
 * NOTE: The caller is responsible for freeing the memory of the returned string
 * using free().
 *
 * @param count The number of strings in the 'strings' array.
 * @param strings An array of C-style strings (char*). Can contain NULL pointers, which are skipped.
 * @return A new dynamically allocated string containing the concatenation.
 *         Returns NULL if memory allocation fails.
 *         Returns an empty, allocated string if count is 0 or strings is NULL.
 */
char* concatenateStrings(int count, const char* const strings[]) {
    if (count <= 0 || strings == NULL) {
        char* empty_str = (char*)malloc(1);
        if (empty_str == NULL) {
            perror("malloc failed for empty string");
            return NULL;
        }
        empty_str[0] = '\0';
        return empty_str;
    }

    // 1. Calculate the total length required
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        if (strings[i] != NULL) { // Safely handle NULL strings in the array
            total_len += strlen(strings[i]);
        }
    }
    
    // 2. Allocate memory for the result string (+1 for the null terminator)
    char* result = (char*)malloc(total_len + 1);
    if (result == NULL) {
        perror("malloc failed for result string");
        return NULL;
    }
    
    // Use a pointer to track the current end of the string for efficient copying
    char* current_pos = result;

    // 3. Copy the strings into the result buffer
    for (int i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            size_t len = strlen(strings[i]);
            memcpy(current_pos, strings[i], len);
            current_pos += len;
        }
    }

    // 4. Add the null terminator at the very end
    *current_pos = '\0';

    return result;
}

void run_test(const char* test_name, const char* expected, int count, const char* const strings[]) {
    printf("%s", test_name);
    char* result = concatenateStrings(count, strings);
    if (result != NULL) {
        printf("%s\n", result);
        printf("Expected: %s\n", expected);
        free(result); // IMPORTANT: Free the allocated memory to prevent leaks
    } else {
        printf("Concatenation failed (memory allocation failed).\n");
    }
    printf("--------------------\n");
}

int main() {
    // Test Case 1: Basic concatenation
    const char* input1[] = {"Hello", ", ", "World", "!"};
    run_test("Test Case 1: ", "Hello, World!", 4, input1);

    // Test Case 2: Concatenating with empty strings
    const char* input2[] = {"One", "", "Two", "", "Three"};
    run_test("Test Case 2: ", "OneTwoThree", 5, input2);

    // Test Case 3: Single string
    const char* input3[] = {"OnlyOneString"};
    run_test("Test Case 3: ", "OnlyOneString", 1, input3);

    // Test Case 4: No strings (count = 0)
    const char* input4[] = {}; // This array is unused when count is 0
    run_test("Test Case 4: ", "", 0, input4);

    // Test Case 5: With NULL pointers in the array
    const char* input5[] = {"C strings", " require ", NULL, "careful", " handling.", NULL};
    run_test("Test Case 5: ", "C strings require careful handling.", 6, input5);

    return 0;
}