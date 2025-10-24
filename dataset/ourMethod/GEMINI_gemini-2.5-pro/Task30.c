#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Takes two strings and returns a new sorted string containing distinct
 *        letters from both input strings.
 * 
 * @param s1 The first input C-string, containing only letters from 'a' to 'z'.
 * @param s2 The second input C-string, containing only letters from 'a' to 'z'.
 * @return A pointer to a new dynamically allocated string. The caller is 
 *         responsible for freeing this memory. Returns NULL on allocation failure.
 */
char* longest(const char* s1, const char* s2) {
    bool seen[26] = {false};
    
    // Process first string
    for (size_t i = 0; s1[i] != '\0'; ++i) {
        if (s1[i] >= 'a' && s1[i] <= 'z') {
            seen[s1[i] - 'a'] = true;
        }
    }

    // Process second string
    for (size_t i = 0; s2[i] != '\0'; ++i) {
        if (s2[i] >= 'a' && s2[i] <= 'z') {
            seen[s2[i] - 'a'] = true;
        }
    }

    // Count the number of unique characters
    int unique_count = 0;
    for (int i = 0; i < 26; ++i) {
        if (seen[i]) {
            unique_count++;
        }
    }

    // Allocate memory for the result string (+1 for null terminator)
    char* result = (char*)malloc(unique_count + 1);
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Build the result string
    int index = 0;
    for (int i = 0; i < 26; ++i) {
        if (seen[i]) {
            result[index++] = (char)('a' + i);
        }
    }
    result[index] = '\0'; // Null-terminate the string

    return result;
}

void run_test(const char* test_name, const char* s1, const char* s2) {
    printf("%s: longest(\"%s\", \"%s\") -> ", test_name, s1, s2);
    char* result = longest(s1, s2);
    if (result) {
        printf("\"%s\"\n", result);
        free(result);
    } else {
        printf("Function failed (e.g., memory allocation error).\n");
    }
}

int main() {
    // Test Case 1
    run_test("Test 1", "xyaabbbccccdefww", "xxxxyyyyabklmopq");

    // Test Case 2
    run_test("Test 2", "abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz");

    // Test Case 3
    run_test("Test 3", "aretheyhere", "yestheyarehere");

    // Test Case 4
    run_test("Test 4", "loopingisfunbutdangerous", "lessdangerousthancoding");

    // Test Case 5
    run_test("Test 5", "", "zyxw");

    return 0;
}