#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Takes 2 strings s1 and s2 including only letters from a to z.
 * Returns a new sorted string, the longest possible, containing distinct letters.
 * 
 * @param s1 The first string.
 * @param s2 The second string.
 * @return A new dynamically allocated string. The caller is responsible for freeing this memory.
 */
char* longest(const char* s1, const char* s2) {
    bool seen[26] = {false}; // for 'a' through 'z'

    // Mark characters from s1 as seen
    for (int i = 0; s1[i] != '\0'; i++) {
        if (s1[i] >= 'a' && s1[i] <= 'z') {
            seen[s1[i] - 'a'] = true;
        }
    }

    // Mark characters from s2 as seen
    for (int i = 0; s2[i] != '\0'; i++) {
        if (s2[i] >= 'a' && s2[i] <= 'z') {
            seen[s2[i] - 'a'] = true;
        }
    }
    
    // Allocate memory for the result string. Max 26 chars + null terminator.
    char* result = (char*)malloc(27 * sizeof(char));
    if (result == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    int index = 0;
    for (int i = 0; i < 26; i++) {
        if (seen[i]) {
            result[index++] = (char)('a' + i);
        }
    }
    result[index] = '\0'; // Null-terminate the string

    return result;
}

int main() {
    char* result;

    // Test Case 1
    const char* a1 = "xyaabbbccccdefww";
    const char* b1 = "xxxxyyyyabklmopq";
    printf("Test Case 1:\n");
    printf("s1: \"%s\"\n", a1);
    printf("s2: \"%s\"\n", b1);
    result = longest(a1, b1);
    printf("Result: \"%s\"\n\n", result); // Expected: "abcdefklmopqwxy"
    free(result);

    // Test Case 2
    const char* a2 = "abcdefghijklmnopqrstuvwxyz";
    const char* b2 = "abcdefghijklmnopqrstuvwxyz";
    printf("Test Case 2:\n");
    printf("s1: \"%s\"\n", a2);
    printf("s2: \"%s\"\n", b2);
    result = longest(a2, b2);
    printf("Result: \"%s\"\n\n", result); // Expected: "abcdefghijklmnopqrstuvwxyz"
    free(result);
    
    // Test Case 3
    const char* a3 = "aretheyhere";
    const char* b3 = "yestheyarehere";
    printf("Test Case 3:\n");
    printf("s1: \"%s\"\n", a3);
    printf("s2: \"%s\"\n", b3);
    result = longest(a3, b3);
    printf("Result: \"%s\"\n\n", result); // Expected: "aehrsty"
    free(result);

    // Test Case 4
    const char* a4 = "loopingisfunbutdangerous";
    const char* b4 = "lessdangerousthancoding";
    printf("Test Case 4:\n");
    printf("s1: \"%s\"\n", a4);
    printf("s2: \"%s\"\n", b4);
    result = longest(a4, b4);
    printf("Result: \"%s\"\n\n", result); // Expected: "abcdefghilnoprstu"
    free(result);

    // Test Case 5
    const char* a5 = "";
    const char* b5 = "";
    printf("Test Case 5:\n");
    printf("s1: \"%s\"\n", a5);
    printf("s2: \"%s\"\n", b5);
    result = longest(a5, b5);
    printf("Result: \"%s\"\n\n", result); // Expected: ""
    free(result);

    return 0;
}