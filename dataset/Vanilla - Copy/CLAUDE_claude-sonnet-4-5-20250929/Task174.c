
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
    
    int sLen = strlen(s);
    char* rev = (char*)malloc(sLen + 1);
    for (int i = 0; i < sLen; i++) {
        rev[i] = s[sLen - 1 - i];
    }
    rev[sLen] = '\\0';
    
    int combinedLen = sLen * 2 + 1;
    char* combined = (char*)malloc(combinedLen + 1);
    sprintf(combined, "%s#%s", s, rev);
    
    int* lps = (int*)malloc(combinedLen * sizeof(int));
    computeLPS(combined, combinedLen, lps);
    
    int longestPalindromePrefix = lps[combinedLen - 1];
    
    int suffixLen = sLen - longestPalindromePrefix;
    char* result = (char*)malloc(sLen + suffixLen + 1);
    
    for (int i = 0; i < suffixLen; i++) {
        result[i] = s[sLen - 1 - i];
    }
    strcpy(result + suffixLen, s);
    
    free(rev);
    free(combined);
    free(lps);
    
    return result;
}

int main() {
    // Test case 1
    char* result1 = shortestPalindrome("aacecaaa");
    printf("Test 1: %s\\n", result1);
    free(result1);
    
    // Test case 2
    char* result2 = shortestPalindrome("abcd");
    printf("Test 2: %s\\n", result2);
    free(result2);
    
    // Test case 3
    char* result3 = shortestPalindrome("");
    printf("Test 3: %s\\n", result3);
    free(result3);
    
    // Test case 4
    char* result4 = shortestPalindrome("a");
    printf("Test 4: %s\\n", result4);
    free(result4);
    
    // Test case 5
    char* result5 = shortestPalindrome("aba");
    printf("Test 5: %s\\n", result5);
    free(result5);
    
    return 0;
}
