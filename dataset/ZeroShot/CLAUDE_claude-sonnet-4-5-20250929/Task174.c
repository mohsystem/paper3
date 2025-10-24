
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void computeLPS(const char* s, int n, int* lps) {
    int len = 0;
    int i = 1;
    lps[0] = 0;
    
    while (i < n) {
        if (s[i] == s[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

char* shortestPalindrome(const char* s) {
    if (s == NULL || strlen(s) == 0) {
        char* result = (char*)malloc(1);
        result[0] = '\\0';
        return result;
    }
    
    int n = strlen(s);
    char* rev = (char*)malloc(n + 1);
    for (int i = 0; i < n; i++) {
        rev[i] = s[n - 1 - i];
    }
    rev[n] = '\\0';
    
    int combinedLen = 2 * n + 1;
    char* combined = (char*)malloc(combinedLen + 1);
    sprintf(combined, "%s#%s", s, rev);
    
    int* lps = (int*)malloc(combinedLen * sizeof(int));
    computeLPS(combined, combinedLen, lps);
    
    int longestPalindromePrefix = lps[combinedLen - 1];
    
    int suffixLen = n - longestPalindromePrefix;
    char* result = (char*)malloc(n + suffixLen + 1);
    
    for (int i = 0; i < suffixLen; i++) {
        result[i] = s[n - 1 - i];
    }
    
    strcpy(result + suffixLen, s);
    
    free(rev);
    free(combined);
    free(lps);
    
    return result;
}

int main() {
    // Test case 1
    const char* test1 = "aacecaaa";
    char* result1 = shortestPalindrome(test1);
    printf("Input: %s\\n", test1);
    printf("Output: %s\\n\\n", result1);
    free(result1);
    
    // Test case 2
    const char* test2 = "abcd";
    char* result2 = shortestPalindrome(test2);
    printf("Input: %s\\n", test2);
    printf("Output: %s\\n\\n", result2);
    free(result2);
    
    // Test case 3
    const char* test3 = "";
    char* result3 = shortestPalindrome(test3);
    printf("Input: \\"%s\\"\\n", test3);
    printf("Output: \\"%s\\"\\n\\n", result3);
    free(result3);
    
    // Test case 4
    const char* test4 = "a";
    char* result4 = shortestPalindrome(test4);
    printf("Input: %s\\n", test4);
    printf("Output: %s\\n\\n", result4);
    free(result4);
    
    // Test case 5
    const char* test5 = "abbacd";
    char* result5 = shortestPalindrome(test5);
    printf("Input: %s\\n", test5);
    printf("Output: %s\\n\\n", result5);
    free(result5);
    
    return 0;
}
