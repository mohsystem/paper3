#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function for max
int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * @brief Given a string containing just the characters '(' and ')', returns the length
 * of the longest valid (well-formed) parentheses substring.
 * 
 * @param s The input string consisting of '(' and ')'.
 * @return The length of the longest valid parentheses substring.
 */
int longestValidParentheses(const char *s) {
    if (s == NULL) {
        return 0;
    }
    size_t n = strlen(s);
    if (n < 2) {
        return 0;
    }

    int maxLen = 0;
    
    // Using a dynamically allocated array as a stack
    // The stack size can be at most n + 1 (for sentinel + all possible '(')
    int *stack = (int *)malloc((n + 1) * sizeof(int));
    if (stack == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        return 0; 
    }
    
    int top = 0; // 'top' is the index for the next element to be pushed
    
    // Push -1 as a sentinel value. It acts as a boundary for the first valid substring.
    stack[top++] = -1; 

    for (size_t i = 0; i < n; i++) {
        if (s[i] == '(') {
            stack[top++] = i;
        } else { // s[i] == ')'
            top--;
            if (top == 0) {
                // Current ')' is unmatched. Push its index as the new boundary.
                stack[top++] = i;
            } else {
                // A valid pair is found. The length is the current index minus the
                // index of the character just before the start of this valid substring.
                maxLen = max(maxLen, (int)i - stack[top - 1]);
            }
        }
    }

    free(stack);
    return maxLen;
}

int main() {
    // Test cases
    const char *s1 = "(()";
    printf("Input: \"%s\", Output: %d\n", s1, longestValidParentheses(s1));

    const char *s2 = ")()())";
    printf("Input: \"%s\", Output: %d\n", s2, longestValidParentheses(s2));

    const char *s3 = "";
    printf("Input: \"%s\", Output: %d\n", s3, longestValidParentheses(s3));
    
    const char *s4 = "()(()";
    printf("Input: \"%s\", Output: %d\n", s4, longestValidParentheses(s4));
    
    const char *s5 = "()(())";
    printf("Input: \"%s\", Output: %d\n", s5, longestValidParentheses(s5));

    return 0;
}