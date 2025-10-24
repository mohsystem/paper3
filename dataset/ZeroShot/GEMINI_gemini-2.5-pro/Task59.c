#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Transforms a string based on the specified accumulation rule.
 * Example: accum("abcd") -> "A-Bb-Ccc-Dddd"
 * @param s The input string, containing only letters from a..z and A..Z.
 * @return A new heap-allocated string with the transformed content. The caller is responsible for freeing this memory.
 *         Returns NULL if memory allocation fails.
 */
char* accum(const char* s) {
    if (s == NULL) {
        s = ""; // Treat NULL input as an empty string for safety.
    }
    
    size_t n = strlen(s);
    if (n == 0) {
        char* result = (char*)malloc(1);
        if (result == NULL) {
            return NULL;
        }
        result[0] = '\0';
        return result;
    }

    // Calculate exact length for allocation:
    // n*(n+1)/2 for letters + (n-1) for hyphens
    size_t len = (n * (n + 1)) / 2 + (n - 1);
    char* result = (char*)malloc(len + 1); // +1 for the null terminator

    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    char* p = result; // Use a pointer to fill the result string efficiently
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) {
            *p++ = '-';
        }
        *p++ = toupper((unsigned char)s[i]);
        for (size_t j = 0; j < i; ++j) {
            *p++ = tolower((unsigned char)s[i]);
        }
    }
    *p = '\0'; // Null-terminate the string

    return result;
}

int main() {
    const char* testCases[] = {
        "abcd",
        "RqaEzty",
        "cwAt",
        "Z",
        ""
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        char* result = accum(testCases[i]);
        if (result != NULL) {
            printf("Input: \"%s\" -> Output: \"%s\"\n", testCases[i], result);
            free(result); // Free the dynamically allocated memory
        } else {
            fprintf(stderr, "Memory allocation failed for input: \"%s\"\n", testCases[i]);
        }
    }

    return 0;
}