#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char* accum(const char* s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = strlen(s);
    if (len == 0) {
        char* empty_str = (char*)malloc(1);
        if (empty_str) {
            empty_str[0] = '\0';
        }
        return empty_str;
    }

    // Calculate length of the result string
    // Sum of lengths of parts: 1+2+...+len = len*(len+1)/2
    // Number of hyphens: len - 1
    size_t result_len = (len * (len + 1)) / 2 + (len - 1);
    char* result = (char*)malloc(result_len + 1); // +1 for null terminator
    if (result == NULL) {
        return NULL; // Allocation failed
    }

    char* p = result; // Pointer to current position in result
    for (size_t i = 0; i < len; ++i) {
        // Append the first character, capitalized
        *p++ = toupper((unsigned char)s[i]);
        // Append the rest of the characters, in lowercase, i times
        for (size_t j = 0; j < i; ++j) {
            *p++ = tolower((unsigned char)s[i]);
        }
        // Append a hyphen if it's not the last part
        if (i < len - 1) {
            *p++ = '-';
        }
    }
    *p = '\0'; // Null-terminate the string

    return result;
}

int main() {
    const char* test_cases[] = {"abcd", "RqaEzty", "cwAt", "Z", ""};
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        char* result = accum(test_cases[i]);
        if (result) {
            printf("\"%s\" -> \"%s\"\n", test_cases[i], result);
            free(result); // Free the allocated memory
        } else {
            fprintf(stderr, "Memory allocation failed for input: \"%s\"\n", test_cases[i]);
        }
    }
    return 0;
}