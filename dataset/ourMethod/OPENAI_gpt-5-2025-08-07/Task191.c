#include <stdio.h>
#include <stdlib.h>

int longestValidParentheses(const char* s) {
    if (s == NULL) {
        return -1;
    }
    size_t n = 0;
    while (s[n] != '\0') {
        char c = s[n];
        if (c != '(' && c != ')') {
            return -1;
        }
        n++;
        if (n > 30000u) {
            return -1;
        }
    }
    if (n == 0u) {
        return 0;
    }
    int* stack = (int*)malloc(sizeof(int) * (n + 1u));
    if (stack == NULL) {
        return -1;
    }
    int top = 0;
    stack[top] = -1;
    int maxLen = 0;
    for (int i = 0; i < (int)n; ++i) {
        if (s[i] == '(') {
            top++;
            stack[top] = i;
        } else {
            top--;
            if (top < 0) {
                top = 0;
                stack[top] = i;
            } else {
                int len = i - stack[top];
                if (len > maxLen) {
                    maxLen = len;
                }
            }
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
        "abc"
    };
    for (int i = 0; i < 5; ++i) {
        int res = longestValidParentheses(tests[i]);
        printf("%d\n", res);
    }
    return 0;
}