#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 Chain-of-Through Process:
 1) Problem understanding: Create a sorted string of unique letters from s1 and s2.
 2) Security requirements: Validate characters, avoid buffer overflows, handle allocation failures.
 3) Secure coding generation: Fixed-size presence array, careful malloc size, bounds-checked indices.
 4) Code review: Use unsigned char for char classification; ensure null-termination and memory management.
 5) Secure code output: Return dynamically allocated string with distinct sorted lowercase letters.
*/

char* longest(const char* s1, const char* s2) {
    bool present[26] = { false };

    // Helper lambda-like function implemented as a local block
    const char* p;

    if (s1 != NULL) {
        p = s1;
        while (*p != '\0') {
            unsigned char uc = (unsigned char)*p++;
            if (uc >= 'A' && uc <= 'Z') uc = (unsigned char)(uc - 'A' + 'a');
            if (uc >= 'a' && uc <= 'z') {
                present[uc - 'a'] = true;
            }
        }
    }
    if (s2 != NULL) {
        p = s2;
        while (*p != '\0') {
            unsigned char uc = (unsigned char)*p++;
            if (uc >= 'A' && uc <= 'Z') uc = (unsigned char)(uc - 'A' + 'a');
            if (uc >= 'a' && uc <= 'z') {
                present[uc - 'a'] = true;
            }
        }
    }

    // Count how many letters are present
    size_t count = 0;
    for (int i = 0; i < 26; ++i) {
        if (present[i]) ++count;
    }

    // Allocate result string (+1 for null terminator)
    char* out = (char*)malloc(count + 1);
    if (out == NULL) {
        return NULL; // Allocation failed
    }

    // Build sorted output
    size_t idx = 0;
    for (int i = 0; i < 26; ++i) {
        if (present[i]) {
            out[idx++] = (char)('a' + i);
        }
    }
    out[idx] = '\0';
    return out;
}

int main(void) {
    const char* tests[5][2] = {
        {"xyaabbbccccdefww", "xxxxyyyyabklmopq"},
        {"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
        {"abc", "def"},
        {"zzz", "zzzz"},
        {"", "abc"}
    };

    for (int i = 0; i < 5; ++i) {
        char* res = longest(tests[i][0], tests[i][1]);
        if (res != NULL) {
            printf("Test %d: %s\n", i + 1, res);
            free(res);
        } else {
            printf("Test %d: (allocation failed)\n", i + 1);
        }
    }
    return 0;
}