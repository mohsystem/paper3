
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Security: Input validation to ensure string contains only '(' and ')'
// Returns false if invalid input detected
bool validateInput(const char* s, size_t len) {
    // Check string length against constraint (0 <= length <= 30000)
    if (len > 30000) {
        return false;
    }
    
    // Security: Null pointer check
    if (s == NULL && len > 0) {
        return false;
    }
    
    // Validate each character is either '(' or ')'
    for (size_t i = 0; i < len; ++i) {
        if (s[i] != '(' && s[i] != ')') {
            return false;
        }
    }
    return true;
}

int longestValidParentheses(const char* s) {
    // Security: Null pointer check
    if (s == NULL) {
        return 0;
    }
    
    // Security: Use strlen safely and store result
    size_t n = strlen(s);
    
    // Security: Validate input before processing
    if (!validateInput(s, n)) {
        return -1; // Invalid input
    }
    
    // Handle empty string case
    if (n == 0) {
        return 0;
    }
    
    // Security: Allocate memory with calloc to initialize to 0
    // Check for allocation failure
    int* dp = (int*)calloc(n, sizeof(int));
    if (dp == NULL) {
        return -1; // Memory allocation failed
    }
    
    int maxLen = 0;
    
    // Dynamic programming approach
    for (size_t i = 1; i < n; ++i) {
        if (s[i] == ')') {
            if (s[i - 1] == '(') {
                // Case: ...()
                // Security: Bounds check before array access
                dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
            } else if (dp[i - 1] > 0) {
                // Case: ...))
                // Security: Calculate match_idx with overflow protection
                size_t dp_prev = (size_t)dp[i - 1];
                
                // Security: Check for underflow in subtraction
                if (i > dp_prev) {
                    size_t match_idx = i - dp_prev - 1;
                    
                    // Security: Bounds check before array access
                    if (match_idx < n && s[match_idx] == '(') {
                        dp[i] = dp[i - 1] + 2;
                        
                        // Security: Bounds check for match_idx - 1
                        if (match_idx >= 1) {
                            dp[i] += dp[match_idx - 1];
                        }
                    }
                }
            }
            
            // Security: Update maxLen safely
            if (dp[i] > maxLen) {
                maxLen = dp[i];
            }
        }
    }
    
    // Security: Free allocated memory before returning
    free(dp);
    dp = NULL;
    
    return maxLen;
}

int main() {
    // Test case 1
    const char* test1 = "(()";
    printf("Input: \\"%s\\" -> Output: %d\\n", test1, longestValidParentheses(test1));
    
    // Test case 2
    const char* test2 = ")()())";
    printf("Input: \\"%s\\" -> Output: %d\\n", test2, longestValidParentheses(test2));
    
    // Test case 3
    const char* test3 = "";
    printf("Input: \\"%s\\" -> Output: %d\\n", test3, longestValidParentheses(test3));
    
    // Test case 4
    const char* test4 = "((()))";
    printf("Input: \\"%s\\" -> Output: %d\\n", test4, longestValidParentheses(test4));
    
    // Test case 5
    const char* test5 = "()(())";
    printf("Input: \\"%s\\" -> Output: %d\\n", test5, longestValidParentheses(test5));
    
    return 0;
}
