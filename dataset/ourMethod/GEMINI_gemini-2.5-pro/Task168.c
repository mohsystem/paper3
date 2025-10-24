#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Tweaks letters in a string according to an integer array.
 *
 * NOTE: The caller is responsible for freeing the returned string using free().
 *
 * @param str The input null-terminated string.
 * @param tweaks An array of integers.
 * @param tweaks_len The number of elements in the tweaks array.
 * @return A new dynamically allocated string with the tweaked letters, or NULL on error.
 *         Errors include NULL inputs, mismatched lengths, or memory allocation failure.
 */
char* tweakLetters(const char* str, const int* tweaks, size_t tweaks_len) {
    if (str == NULL || tweaks == NULL) {
        // Handle case where empty string is passed with 0 length array
        if (str != NULL && strlen(str) == 0 && tweaks_len == 0) {
            // fall through to allow empty string processing
        } else {
            return NULL;
        }
    }

    size_t str_len = strlen(str);
    if (str_len != tweaks_len) {
        return NULL; // Mismatched lengths
    }

    char* result = (char*)malloc(str_len + 1);
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    for (size_t i = 0; i < str_len; ++i) {
        result[i] = (char)(str[i] + tweaks[i]);
    }
    result[str_len] = '\0';

    return result;
}

void run_test_case(const char* test_name, const char* str, int* tweaks, size_t tweaks_len) {
    char* result = tweakLetters(str, tweaks, tweaks_len);
    printf("Test: %s -> ", test_name);
    if (result) {
        printf("\"%s\"\n", result);
        free(result);
    } else {
        printf("NULL (Error)\n");
    }
}

int main() {
    // Test Case 1
    int tweaks1[] = {0, 1, -1, 0, -1};
    run_test_case("'apple', {0, 1, -1, 0, -1}", "apple", tweaks1, sizeof(tweaks1) / sizeof(int));

    // Test Case 2
    int tweaks2[] = {0, 0, 0, -1};
    run_test_case("'many', {0, 0, 0, -1}", "many", tweaks2, sizeof(tweaks2) / sizeof(int));

    // Test Case 3
    int tweaks3[] = {1, 1, 1, 1, 1};
    run_test_case("'rhino', {1, 1, 1, 1, 1}", "rhino", tweaks3, sizeof(tweaks3) / sizeof(int));

    // Test Case 4: Empty string
    int tweaks4[] = {};
    run_test_case("'', {}", "", tweaks4, 0);

    // Test Case 5: All zeros
    int tweaks5[] = {0, 0, 0, 0, 0};
    run_test_case("'hello', {0, 0, 0, 0, 0}", "hello", tweaks5, sizeof(tweaks5) / sizeof(int));

    return 0;
}