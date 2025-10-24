#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Computes the Longest Proper Prefix Suffix (LPS) array for KMP algorithm.
 * 
 * @param pattern The string for which to compute the LPS array.
 * @param lps Pointer to the integer array to store LPS values.
 * @param n Length of the pattern string.
 */
void computeLPSArray(const char* pattern, int* lps, int n) {
    lps[0] = 0;
    int length = 0;
    int i = 1;
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

/**
 * @brief Finds the shortest palindrome by adding characters to the front of the string.
 * 
 * @param s The input string.
 * @return char* The shortest palindrome. The caller must free this memory.
 */
char* shortestPalindrome(const char* s) {
    int n = strlen(s);
    if (n <= 1) {
        char* res = (char*)malloc(n + 1);
        if (!res) return NULL;
        memcpy(res, s, n + 1);
        return res;
    }

    char* rev_s = (char*)malloc(n + 1);
    if (!rev_s) return NULL;
    for (int i = 0; i < n; i++) {
        rev_s[i] = s[n - 1 - i];
    }
    rev_s[n] = '\0';

    int temp_len = 2 * n + 1;
    char* temp = (char*)malloc(temp_len + 1);
    if (!temp) {
        free(rev_s);
        return NULL;
    }
    snprintf(temp, temp_len + 1, "%s#%s", s, rev_s);

    int* lps = (int*)malloc(temp_len * sizeof(int));
    if (!lps) {
        free(rev_s);
        free(temp);
        return NULL;
    }
    computeLPSArray(temp, lps, temp_len);
    
    int lpsLength = lps[temp_len - 1];
    int to_add_len = n - lpsLength;

    char* result = (char*)malloc(to_add_len + n + 1);
    if (!result) {
        free(rev_s);
        free(temp);
        free(lps);
        return NULL;
    }

    for (int i = 0; i < to_add_len; i++) {
        result[i] = s[n - 1 - i];
    }
    memcpy(result + to_add_len, s, n);
    result[to_add_len + n] = '\0';

    free(rev_s);
    free(temp);
    free(lps);

    return result;
}

int main() {
    const char* testCases[] = {
        "aacecaaa",
        "abcd",
        "aba",
        "",
        "a"
    };
    const char* expectedResults[] = {
        "aaacecaaa",
        "dcbabcd",
        "aba",
        "",
        "a"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        const char* s = testCases[i];
        const char* expected = expectedResults[i];
        char* result = shortestPalindrome(s);

        printf("Input: \"%s\"\n", s);
        if (result) {
            printf("Output: \"%s\"\n", result);
            printf("Expected: \"%s\"\n", expected);
            printf("Test %d %s\n", i + 1, strcmp(result, expected) == 0 ? "Passed" : "Failed");
            free(result);
        } else {
            printf("Output: NULL (Memory allocation failed)\n");
            printf("Test %d Failed\n", i + 1);
        }
        printf("\n");
    }

    return 0;
}