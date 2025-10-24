#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Tweaks letters in a string based on an array of integer shifts.
 * 
 * @param str The input null-terminated string.
 * @param tweaks An array of integers representing the shift for each character.
 * @param len The length of the string and the tweaks array.
 * @return A new dynamically allocated string with the tweaked letters. 
 *         Returns NULL on error. The caller is responsible for freeing this memory.
 */
char* tweakLetters(const char* str, const int* tweaks, size_t len) {
    if (str == NULL || tweaks == NULL) {
        return NULL; // Indicate error with NULL return
    }
    
    // Allocate memory for the result string. +1 for the null terminator.
    char* result = (char*)malloc(sizeof(char) * (len + 1));
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    for (size_t i = 0; i < len; ++i) {
        result[i] = (char)(str[i] + tweaks[i]);
    }
    result[len] = '\0'; // Null-terminate the string

    return result;
}

void run_test_case(const char* str, int tweaks[], size_t len) {
    // Basic check to ensure string length matches array length for the test
    if (strlen(str) != len) {
        printf("Test case error: string length and tweaks length mismatch for \"%s\"\n", str);
        return;
    }
    char* tweaked_str = tweakLetters(str, tweaks, len);
    if (tweaked_str != NULL) {
        printf("%s\n", tweaked_str);
        free(tweaked_str); // Free the allocated memory
    } else {
        printf("Error processing string: %s\n", str);
    }
}

int main() {
    // Test Case 1
    int tweaks1[] = {0, 1, -1, 0, -1};
    run_test_case("apple", tweaks1, sizeof(tweaks1)/sizeof(int));

    // Test Case 2
    int tweaks2[] = {0, 0, 0, -1};
    run_test_case("many", tweaks2, sizeof(tweaks2)/sizeof(int));
    
    // Test Case 3
    int tweaks3[] = {1, 1, 1, 1, 1};
    run_test_case("rhino", tweaks3, sizeof(tweaks3)/sizeof(int));
    
    // Test Case 4
    int tweaks4[] = {-1, 0, 1};
    run_test_case("abc", tweaks4, sizeof(tweaks4)/sizeof(int));
    
    // Test Case 5
    int tweaks5[] = {1};
    run_test_case("z", tweaks5, sizeof(tweaks5)/sizeof(int));

    return 0;
}