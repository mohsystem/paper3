
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Security: Input validation - ensure string length is within constraints
// and contains only lowercase English letters
bool isValidInput(const char* s) {
    if (s == NULL) {
        return false;
    }
    
    size_t len = strlen(s);
    
    // Check length constraint: 0 <= s.length <= 5 * 10^4
    if (len > 50000) {
        return false;
    }
    
    // Validate that string contains only lowercase English letters
    for (size_t i = 0; i < len; i++) {
        if (s[i] < 'a' || s[i] > 'z') {
            return false;
        }
    }
    return true;
}

// Security: Safe string reversal with bounds checking
char* reverseString(const char* s) {
    if (s == NULL) {
        return NULL;
    }
    
    size_t len = strlen(s);
    
    // Security: Check for allocation failure
    char* rev = (char*)malloc(len + 1);
    if (rev == NULL) {
        return NULL;
    }
    
    // Security: Bounds-checked copy in reverse
    for (size_t i = 0; i < len; i++) {
        rev[i] = s[len - 1 - i];
    }
    rev[len] = '\\0'; // Security: Null-terminate string
    
    return rev;
}

// KMP algorithm to compute LPS array
// Security: All allocations checked, bounds validated
int* computeLPSArray(const char* pattern, size_t n) {
    if (pattern == NULL || n == 0) {
        return NULL;
    }
    
    // Security: Check for allocation failure
    int* lps = (int*)calloc(n, sizeof(int));
    if (lps == NULL) {
        return NULL;
    }
    
    int len = 0;
    size_t i = 1;
    
    // Security: Loop bounds checked against array size
    while (i < n) {
        if (pattern[i] == pattern[len]) {
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
    
    return lps;
}

char* shortestPalindrome(const char* s) {
    // Security: Input validation
    if (s == NULL || !isValidInput(s)) {
        return NULL;
    }
    
    size_t n = strlen(s);
    
    // Edge case: empty string or single character
    if (n <= 1) {
        // Security: Allocate and copy safely
        char* result = (char*)malloc(n + 1);
        if (result == NULL) {
            return NULL;
        }
        if (n > 0) {
            strncpy(result, s, n);
        }
        result[n] = '\\0';
        return result;
    }
    
    // Create reverse of string
    char* rev = reverseString(s);
    if (rev == NULL) {
        return NULL;
    }
    
    // Create combined string: s + "#" + reverse(s)
    // Security: Check for potential overflow
    size_t combined_len = n + 1 + n;
    if (combined_len < n) { // Overflow check
        free(rev);
        return NULL;
    }
    
    // Security: Safe allocation with size check
    char* combined = (char*)malloc(combined_len + 1);
    if (combined == NULL) {
        free(rev);
        return NULL;
    }
    
    // Security: Safe string construction with bounds checking
    strncpy(combined, s, n);
    combined[n] = '#';
    strncpy(combined + n + 1, rev, n);
    combined[combined_len] = '\\0';
    
    // Compute LPS array
    int* lps = computeLPSArray(combined, combined_len);
    if (lps == NULL) {
        free(rev);
        free(combined);
        return NULL;
    }
    
    int palindromeLen = lps[combined_len - 1];
    
    // Security: Validate bounds
    if (palindromeLen < 0 || (size_t)palindromeLen > n) {
        free(rev);
        free(combined);
        free(lps);
        return NULL;
    }
    
    size_t charsToAdd = n - palindromeLen;
    
    // Security: Validate calculation
    if (charsToAdd > n) {
        free(rev);
        free(combined);
        free(lps);
        return NULL;
    }
    
    // Create result string
    size_t result_len = charsToAdd + n;
    // Security: Check for overflow
    if (result_len < n) {
        free(rev);
        free(combined);
        free(lps);
        return NULL;
    }
    
    char* result = (char*)malloc(result_len + 1);
    if (result == NULL) {
        free(rev);
        free(combined);
        free(lps);
        return NULL;
    }
    
    // Security: Safe copy with bounds checking
    if (charsToAdd > 0) {
        strncpy(result, rev, charsToAdd);
    }
    strncpy(result + charsToAdd, s, n);
    result[result_len] = '\\0';
    
    // Security: Free all allocated memory
    free(rev);
    free(combined);
    free(lps);
    
    return result;
}

int main() {
    // Test case 1
    const char* test1 = "aacecaaa";
    char* result1 = shortestPalindrome(test1);
    printf("Input: \\"%s\\"\\n", test1);
    printf("Output: \\"%s\\"\\n\\n", result1 ? result1 : "NULL");
    if (result1) free(result1);
    
    // Test case 2
    const char* test2 = "abcd";
    char* result2 = shortestPalindrome(test2);
    printf("Input: \\"%s\\"\\n", test2);
    printf("Output: \\"%s\\"\\n\\n", result2 ? result2 : "NULL");
    if (result2) free(result2);
    
    // Test case 3: empty string
    const char* test3 = "";
    char* result3 = shortestPalindrome(test3);
    printf("Input: \\"%s\\"\\n", test3);
    printf("Output: \\"%s\\"\\n\\n", result3 ? result3 : "NULL");
    if (result3) free(result3);
    
    // Test case 4: single character
    const char* test4 = "a";
    char* result4 = shortestPalindrome(test4);
    printf("Input: \\"%s\\"\\n", test4);
    printf("Output: \\"%s\\"\\n\\n", result4 ? result4 : "NULL");
    if (result4) free(result4);
    
    // Test case 5: already a palindrome
    const char* test5 = "aba";
    char* result5 = shortestPalindrome(test5);
    printf("Input: \\"%s\\"\\n", test5);
    printf("Output: \\"%s\\"\\n\\n", result5 ? result5 : "NULL");
    if (result5) free(result5);
    
    return 0;
}
