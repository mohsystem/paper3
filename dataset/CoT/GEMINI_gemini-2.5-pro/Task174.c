#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Computes the Longest Proper Prefix Suffix (LPS) array for KMP algorithm.
 * @param pattern The string for which to compute the LPS array.
 * @param lps An allocated integer array to store the LPS values.
 * @param n The length of the pattern.
 */
void computeLPSArray(const char* pattern, int* lps, int n) {
    if (n == 0) return;
    lps[0] = 0;
    int length = 0; // length of the previous longest prefix suffix
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
 * Finds the shortest palindrome by adding characters to the front of the string.
 * The caller is responsible for freeing the returned string.
 * @param s The input string.
 * @return The shortest palindrome as a new dynamically allocated string.
 */
char* shortestPalindrome(const char* s) {
    int s_len = strlen(s);
    if (s_len <= 1) {
        char* result = (char*)malloc(s_len + 1);
        if (result) strcpy(result, s);
        return result;
    }

    // Create reversed string
    char* reversed_s = (char*)malloc(s_len + 1);
    if (!reversed_s) return NULL;
    for (int i = 0; i < s_len; i++) {
        reversed_s[i] = s[s_len - 1 - i];
    }
    reversed_s[s_len] = '\0';

    // Create temp string: s + '#' + reversed_s
    int temp_len = 2 * s_len + 1;
    char* temp = (char*)malloc(temp_len + 1);
    if (!temp) {
        free(reversed_s);
        return NULL;
    }
    strcpy(temp, s);
    strcat(temp, "#");
    strcat(temp, reversed_s);
    free(reversed_s);

    // Compute LPS array
    int* lps = (int*)malloc(temp_len * sizeof(int));
    if (!lps) {
        free(temp);
        return NULL;
    }
    computeLPSArray(temp, lps, temp_len);
    free(temp);

    int palindromicPrefixLength = lps[temp_len - 1];
    free(lps);

    int suffix_len = s_len - palindromicPrefixLength;
    
    // Create prefix to prepend (reversed suffix)
    char* prefix = (char*)malloc(suffix_len + 1);
    if (!prefix) return NULL;
    for (int i = 0; i < suffix_len; i++) {
        prefix[i] = s[s_len - 1 - i];
    }
    prefix[suffix_len] = '\0';
    
    // Create final result string
    char* result = (char*)malloc(suffix_len + s_len + 1);
    if (!result) {
        free(prefix);
        return NULL;
    }
    strcpy(result, prefix);
    strcat(result, s);
    free(prefix);

    return result;
}

int main() {
    // Test cases
    const char* test_cases[] = {"aacecaaa", "abcd", "abacaba", "", "abab"};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* s = test_cases[i];
        char* result = shortestPalindrome(s);
        if (result) {
            printf("Input: \"%s\", Output: \"%s\"\n", s, result);
            free(result);
        } else {
            printf("Memory allocation failed for input: %s\n", s);
        }
    }

    return 0;
}