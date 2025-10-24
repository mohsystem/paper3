#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int longestValidParentheses(const char* s) {
    if (s == NULL) return 0;
    size_t n = strlen(s);
    if (n == 0) return 0;

    int* stack = (int*)malloc((n + 1) * sizeof(int));
    if (stack == NULL) return 0; /* allocation failed, return safe default */

    int top = -1;
    int maxLen = 0;

    stack[++top] = -1;
    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (c == '(') {
            stack[++top] = (int)i;
        } else if (c == ')') {
            if (top >= 0) --top; /* pop */
            if (top == -1) {
                stack[++top] = (int)i;
            } else {
                int len = (int)i - stack[top];
                if (len > maxLen) maxLen = len;
            }
        }
    }

    free(stack);
    return maxLen;
}

int main(void) {
    const char* tests[] = {
        "(()",
        ")()())",
        "",
        "()(())",
        "((((("
    };
    size_t count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < count; ++i) {
        printf("%d\n", longestValidParentheses(tests[i]));
    }
    return 0;
}