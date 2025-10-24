#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void computeLPSArray(const char* pattern, int* lps) {
    int n = strlen(pattern);
    int length = 0;
    int i = 1;
    lps[0] = 0;

    while (i < n) {
        if (pattern[i] == pattern[length]) {
            length++;
            lps[i] = length;
            i++;
        } else {
            if (length != 0) {
                length = lps[length - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

char* shortestPalindrome(const char* s) {
    int n = strlen(s);
    if (n <= 1) {
        char* res = (char*)malloc(n + 1);
        strcpy(res, s);
        return res;
    }

    // Create reversed string
    char* rev_s = (char*)malloc(n + 1);
    if (!rev_s) return NULL;
    for (int i = 0; i < n; i++) {
        rev_s[i] = s[n - 1 - i];
    }
    rev_s[n] = '\0';

    // Create temp string: s + '#' + rev_s
    int temp_len = 2 * n + 1;
    char* temp = (char*)malloc(temp_len + 1);
    if (!temp) { free(rev_s); return NULL; }
    strcpy(temp, s);
    strcat(temp, "#");
    strcat(temp, rev_s);

    // Compute LPS array
    int* lps = (int*)malloc(temp_len * sizeof(int));
    if (!lps) { free(rev_s); free(temp); return NULL; }
    computeLPSArray(temp, lps);
    int l = lps[temp_len - 1];

    // Create prefix to add
    int prefix_len = n - l;
    
    // Create result string
    char* result = (char*)malloc(prefix_len + n + 1);
    if (!result) { free(rev_s); free(temp); free(lps); return NULL; }
    strncpy(result, rev_s, prefix_len);
    result[prefix_len] = '\0';
    strcat(result, s);

    // Free intermediate allocated memory
    free(rev_s);
    free(temp);
    free(lps);
    
    return result;
}

int main() {
    const char* test_cases[] = {"aacecaaa", "abcd", "ababa", "race", "a"};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* s = test_cases[i];
        char* res = shortestPalindrome(s);
        printf("Input: %s\n", s);
        printf("Output: %s\n\n", res);
        free(res);
    }

    return 0;
}