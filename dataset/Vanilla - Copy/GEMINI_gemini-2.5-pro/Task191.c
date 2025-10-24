#include <stdio.h>
#include <string.h>

int max(int a, int b) {
    return a > b ? a : b;
}

int longestValidParentheses(char* s) {
    int left = 0, right = 0, maxLength = 0;
    int n = strlen(s);
    
    // Left to Right scan
    for (int i = 0; i < n; ++i) {
        if (s[i] == '(') {
            left++;
        } else {
            right++;
        }
        if (left == right) {
            maxLength = max(maxLength, 2 * right);
        } else if (right > left) {
            left = 0;
            right = 0;
        }
    }
    
    left = 0;
    right = 0;
    
    // Right to Left scan
    for (int i = n - 1; i >= 0; --i) {
        if (s[i] == '(') {
            left++;
        } else {
            right++;
        }
        if (left == right) {
            maxLength = max(maxLength, 2 * left);
        } else if (left > right) {
            left = 0;
            right = 0;
        }
    }
    
    return maxLength;
}

int main() {
    // Test Case 1
    char s1[] = "(()";
    printf("Input: %s, Output: %d\n", s1, longestValidParentheses(s1));

    // Test Case 2
    char s2[] = ")()())";
    printf("Input: %s, Output: %d\n", s2, longestValidParentheses(s2));

    // Test Case 3
    char s3[] = "";
    printf("Input: %s, Output: %d\n", s3, longestValidParentheses(s3));

    // Test Case 4
    char s4[] = "()(()";
    printf("Input: %s, Output: %d\n", s4, longestValidParentheses(s4));
    
    // Test Case 5
    char s5[] = "()(())";
    printf("Input: %s, Output: %d\n", s5, longestValidParentheses(s5));

    return 0;
}