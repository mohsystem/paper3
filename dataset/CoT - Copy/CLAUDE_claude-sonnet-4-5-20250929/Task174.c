
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void computeLPS(const char* s, int* lps, int n) {
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
    // Input validation
    if (s == NULL || strlen(s) == 0) {
        char* result = (char*)malloc(1);
        if (result) result[0] = '\\0';
        return result;
    }
    
    int len = strlen(s);
    if (len > 50000) {
        return NULL;
    }
    
    // Validate that string contains only lowercase letters
    for (int i = 0; i < len; i++) {
        if (s[i] < 'a' || s[i] > 'z') {
            return NULL;
        }
    }
    
    // Create reversed string
    char* rev = (char*)malloc(len + 1);
    if (!rev) return NULL;
    
    for (int i = 0; i < len; i++) {
        rev[i] = s[len - 1 - i];
    }
    rev[len] = '\\0';
    
    // Create combined string
    int combinedLen = 2 * len + 1;
    char* combined = (char*)malloc(combinedLen + 1);
    if (!combined) {
        free(rev);
        return NULL;
    }
    
    sprintf(combined, "%s#%s", s, rev);
    
    // Compute LPS array
    int* lps = (int*)malloc(combinedLen * sizeof(int));
    if (!lps) {
        free(rev);
        free(combined);
        return NULL;
    }
    
    computeLPS(combined, lps, combinedLen);
    
    int palindromeLength = lps[combinedLen - 1];
    int prependLen = len - palindromeLength;
    
    // Create result
    char* result = (char*)malloc(len + prependLen + 1);
    if (!result) {
        free(rev);
        free(combined);
        free(lps);
        return NULL;
    }
    
    strncpy(result, rev, prependLen);
    strcpy(result + prependLen, s);
    
    free(rev);
    free(combined);
    free(lps);
    
    return result;
}

int main() {
    // Test cases
    char* result;
    
    result = shortestPalindrome("aacecaaa");
    if (result) { printf("%s\\n", result); free(result); }
    
    result = shortestPalindrome("abcd");
    if (result) { printf("%s\\n", result); free(result); }
    
    result = shortestPalindrome("");
    if (result) { printf("%s\\n", result); free(result); }
    
    result = shortestPalindrome("a");
    if (result) { printf("%s\\n", result); free(result); }
    
    result = shortestPalindrome("abbacd");
    if (result) { printf("%s\\n", result); free(result); }
    
    return 0;
}
