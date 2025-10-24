
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int max(int a, int b) {
    return a > b ? a : b;
}

int longestValidParentheses(char* s) {
    if (s == NULL || strlen(s) == 0) {
        return 0;
    }
    
    int len = strlen(s);
    int maxLen = 0;
    int* dp = (int*)calloc(len, sizeof(int));
    
    for (int i = 1; i < len; i++) {
        if (s[i] == ')') {
            if (s[i - 1] == '(') {
                dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
            } else if (i - dp[i - 1] > 0 && s[i - dp[i - 1] - 1] == '(') {
                dp[i] = dp[i - 1] + 2 + (i - dp[i - 1] >= 2 ? dp[i - dp[i - 1] - 2] : 0);
            }
            maxLen = max(maxLen, dp[i]);
        }
    }
    
    free(dp);
    return maxLen;
}

int main() {
    // Test case 1
    printf("Test 1: %d\\n", longestValidParentheses("(()"));
    
    // Test case 2
    printf("Test 2: %d\\n", longestValidParentheses(")()())"));
    
    // Test case 3
    printf("Test 3: %d\\n", longestValidParentheses(""));
    
    // Test case 4
    printf("Test 4: %d\\n", longestValidParentheses("()(())"));
    
    // Test case 5
    printf("Test 5: %d\\n", longestValidParentheses("(()())"));
    
    return 0;
}
