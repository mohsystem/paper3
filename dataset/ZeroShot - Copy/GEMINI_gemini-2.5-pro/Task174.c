#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to reverse a portion of a string
void reverse_string(char *str, int len) {
    int i = 0;
    int j = len - 1;
    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

// Computes the Longest Proper Prefix Suffix (LPS) array for KMP algorithm.
int* computeLPS(const char* pattern) {
    int n = strlen(pattern);
    // Use calloc for zero-initialized memory, which is safer.
    int* lps = (int*)calloc(n, sizeof(int));
    if (!lps) return NULL; // Check for allocation failure

    int length = 0; // lps[0] is already 0
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
    return lps;
}

/**
 * Finds the shortest palindrome by adding characters in front of the string.
 * NOTE: The returned string is dynamically allocated and must be freed by the caller.
 *
 * @param s The input string.
 * @return A new dynamically allocated string which is the shortest palindrome.
 *         Returns NULL on allocation failure.
 */
char* shortestPalindrome(const char* s) {
    if (s == NULL) {
        char* res = (char*)malloc(1);
        if (res) res[0] = '\0';
        return res;
    }

    int n = strlen(s);
    if (n <= 1) {
        char* res = (char*)malloc(n + 1);
        if (!res) return NULL;
        strcpy(res, s);
        return res;
    }

    // 1. Create reversed string
    char* rev_s = (char*)malloc(n + 1);
    if (!rev_s) return NULL;
    strcpy(rev_s, s);
    reverse_string(rev_s, n);

    // 2. Create temp string: s + "#" + rev_s
    int temp_len = 2 * n + 1;
    char* temp = (char*)malloc(temp_len + 1);
    if (!temp) {
        free(rev_s);
        return NULL;
    }
    strcpy(temp, s);
    strcat(temp, "#");
    strcat(temp, rev_s);

    // 3. Compute LPS array
    int* lps = computeLPS(temp);
    if (!lps) {
        free(rev_s);
        free(temp);
        return NULL;
    }
    
    int lps_len = (temp_len > 0) ? lps[temp_len - 1] : 0;
    
    // 4. Determine prefix to add
    int suffix_len = n - lps_len;
    char* prefix_to_add = (char*)malloc(suffix_len + 1);
    if (!prefix_to_add) {
        free(rev_s);
        free(temp);
        free(lps);
        return NULL;
    }
    // strncpy is safer as it avoids buffer overflows
    strncpy(prefix_to_add, s + lps_len, suffix_len);
    prefix_to_add[suffix_len] = '\0'; // Ensure null-termination
    reverse_string(prefix_to_add, suffix_len);

    // 5. Construct the final result
    int res_len = suffix_len + n;
    char* result = (char*)malloc(res_len + 1);
    if (!result) {
        free(rev_s);
        free(temp);
        free(lps);
        free(prefix_to_add);
        return NULL;
    }
    strcpy(result, prefix_to_add);
    strcat(result, s);

    // 6. Free all intermediate memory
    free(rev_s);
    free(temp);
    free(lps);
    free(prefix_to_add);

    return result;
}

int main() {
    const char* test_cases[] = {"aacecaaa", "abcd", "a", "", "abacaba"};
    const char* expected_results[] = {"aaacecaaa", "dcbabcd", "a", "", "abacaba"};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* s = test_cases[i];
        const char* expected = expected_results[i];
        printf("Input: \"%s\"\n", s);
        char* result = shortestPalindrome(s);
        
        if (result) {
            printf("Output: \"%s\"\n", result);
            printf("Expected: \"%s\"\n", expected);
            printf("%s\n", strcmp(result, expected) == 0 ? "Test PASSED" : "Test FAILED");
            free(result); // Free the memory allocated by shortestPalindrome
        } else {
            printf("Error: Memory allocation failed.\n");
        }
        printf("--------------------\n");
    }
    
    return 0;
}