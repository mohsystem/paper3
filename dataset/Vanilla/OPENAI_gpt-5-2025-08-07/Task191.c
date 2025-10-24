#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int longestValidParentheses(const char* s) {
    int n = (int)strlen(s);
    if (n == 0) return 0;
    int* stack = (int*)malloc(sizeof(int) * (n + 1));
    int top = -1;
    int maxLen = 0;

    // push -1 as base index
    stack[++top] = -1;

    for (int i = 0; i < n; ++i) {
        if (s[i] == '(') {
            stack[++top] = i;
        } else {
            if (top >= 0) top--; // pop
            if (top < 0) {
                stack[++top] = i; // push current index as new base
            } else {
                int len = i - stack[top];
                if (len > maxLen) maxLen = len;
            }
        }
    }

    free(stack);
    return maxLen;
}

int main() {
    const char* tests[5] = {
        "(()",
        ")()())",
        "",
        "()(())",
        "())((())"
    };
    for (int i = 0; i < 5; ++i) {
        printf("Input: \"%s\" => Output: %d\n", tests[i], longestValidParentheses(tests[i]));
    }
    return 0;
}