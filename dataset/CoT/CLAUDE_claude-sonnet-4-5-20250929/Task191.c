
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int longestValidParentheses(const char* s) {
    // Input validation
    if (s == NULL || strlen(s) < 2) {
        return 0;
    }
    
    int len = strlen(s);
    int maxLen = 0;
    int* dp = (int*)calloc(len, sizeof(int));
    
    if (dp == NULL) {
        return 0;
    }
    
    for (int i = 1; i < len; i++) {
        if (s[i] == ')') {
            if (s[i - 1] == '(') {
                // Case: ...()
                dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
            } else if (i - dp[i - 1] > 0 && s[i - dp[i - 1] - 1] == '(') {
                // Case: ...))
                dp[i] = dp[i - 1] + 2 + (i - dp[i - 1] >= 2 ? dp[i - dp[i - 1] - 2] : 0);
            }
            if (dp[i] > maxLen) {
                maxLen = dp[i];
            }
        }
    }
    
    free(dp);
    return maxLen;
}

int main() {
    // Test cases
    printf("Test 1: %d (Expected: 2)\\n", longestValidParentheses("(()"));
    printf("Test 2: %d (Expected: 4)\\n", longestValidParentheses(")()())"));
    printf("Test 3: %d (Expected: 0)\\n", longestValidParentheses(""));
    printf("Test 4: %d (Expected: 2)\\n", longestValidParentheses("()(()"));
    printf("Test 5: %d (Expected: 6)\\n", longestValidParentheses("(()())"));
    
    return 0;
}
