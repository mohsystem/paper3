#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Takes two strings s1 and s2 including only letters from a to z.
 * Returns a new sorted string, the longest possible, containing distinct letters.
 * The caller is responsible for freeing the returned string.
 */
char* longest(const char* s1, const char* s2) {
    // Frequency array to mark presence of characters 'a' through 'z'
    int present[26] = {0};
    int unique_count = 0;
    
    // Process first string
    for (int i = 0; s1[i] != '\0'; i++) {
        if (s1[i] >= 'a' && s1[i] <= 'z') {
            present[s1[i] - 'a'] = 1;
        }
    }
    
    // Process second string
    for (int i = 0; s2[i] != '\0'; i++) {
        if (s2[i] >= 'a' && s2[i] <= 'z') {
            present[s2[i] - 'a'] = 1;
        }
    }
    
    // Count unique characters to determine memory size
    for (int i = 0; i < 26; i++) {
        if (present[i]) {
            unique_count++;
        }
    }
    
    // Allocate memory for the result string (+1 for null terminator)
    char* result = (char*)malloc(unique_count + 1);
    if (result == NULL) {
        // Handle memory allocation failure
        return NULL;
    }
    
    // Build the result string
    int index = 0;
    for (int i = 0; i < 26; i++) {
        if (present[i]) {
            result[index++] = (char)('a' + i);
        }
    }
    result[index] = '\0'; // Null-terminate the string
    
    return result;
}

void run_test_case(int test_num, const char* s1, const char* s2) {
    char* result = longest(s1, s2);
    if (result) {
        printf("Test %d: %s\n", test_num, result);
        free(result); // Free the allocated memory
    } else {
        fprintf(stderr, "Test %d: Memory allocation failed.\n", test_num);
    }
}

int main() {
    printf("C Test Cases:\n");

    // Test Case 1
    run_test_case(1, "xyaabbbccccdefww", "xxxxyyyyabklmopq");

    // Test Case 2
    run_test_case(2, "abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz");

    // Test Case 3
    run_test_case(3, "aretheyhere", "yestheyarehere");

    // Test Case 4
    run_test_case(4, "loopingisfunbutdangerous", "lessdangerousthancoding");

    // Test Case 5
    run_test_case(5, "inmanylanguages", "theresapairoffunctions");
    
    return 0;
}