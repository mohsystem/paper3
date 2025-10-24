#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int longestValidParentheses(const char* s) {
    if (s == NULL) return 0;
    size_t n = strlen(s);
    if (n == 0) return 0;

    int* stack = (int*)malloc((n + 1) * sizeof(int));
    if (stack == NULL) {
        return 0;
    }

    int top = 0;
    stack[top++] = -1;
    int maxLen = 0;

    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (c == '(') {
            stack[top++] = (int)i;
        } else if (c == ')') {
            if (top > 0) {
                top--;
            }
            if (top == 0) {
                stack[top++] = (int)i;
            } else {
                int len = (int)i - stack[top - 1];
                if (len > maxLen) {
                    maxLen = len;
                }
            }
        } else {
            // Ignore unexpected characters
        }
    }

    free(stack);
    return maxLen;
}

int main(void) {
    const char* tests[5] = {
        "(()",
        ")()())",
        "",
        "()(())",
        "())((())"
    };
    for (int i = 0; i < 5; ++i) {
        printf("Input: \"%s\" -> %d\n", tests[i], longestValidParentheses(tests[i]));
    }
    return 0;
}