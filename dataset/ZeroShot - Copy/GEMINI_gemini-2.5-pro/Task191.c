#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to find the maximum of two integers.
int max(int a, int b) {
    return a > b ? a : b;
}

/**
 * @brief Given a string containing just the characters '(' and ')', returns the length of the
 * longest valid (well-formed) parentheses substring.
 * 
 * @param s The input string consisting of '(' and ')'.
 * @return The length of the longest valid parentheses substring.
 */
int longestValidParentheses(const char* s) {
    if (s == NULL) {
        return 0;
    }
    int n = strlen(s);
    if (n < 2) {
        return 0;
    }

    // Using a dynamically allocated array as a stack to store indices.
    // Allocate enough space for all indices plus the initial sentinel value.
    int* stack = (int*)malloc((n + 1) * sizeof(int));
    if (stack == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0; // Fail gracefully
    }
    
    int top = 0;
    stack[top] = -1; // Sentinel value to mark the base for a valid substring.
    int maxLength = 0;

    for (int i = 0; i < n; i++) {
        if (s[i] == '(') {
            stack[++top] = i;
        } else { // s[i] == ')'
            // Pop by moving the top pointer down. The value is not overwritten
            // but becomes inaccessible.
            top--;
            if (top < 0) {
                // Stack became empty after pop, meaning this ')' has no matching '('.
                // It now becomes the new "base" for the next valid substring.
                // Reset the stack with this new base index.
                top = 0;
                stack[top] = i;
            } else {
                // A valid pair is found.
                // The length is the current index minus the index of the element
                // at the top of the stack (which is the new base).
                maxLength = max(maxLength, i - stack[top]);
            }
        }
    }

    free(stack);
    return maxLength;
}

void runTestCase(const char* s, int expected) {
    int result = longestValidParentheses(s);
    printf("Test Case: \"%s\"\n", s);
    printf("Result: %d\n", result);
    printf("Expected: %d\n", expected);
    printf("%s\n\n", result == expected ? "Passed" : "Failed");
}

int main() {
    runTestCase("(()", 2);
    runTestCase(")()())", 4);
    runTestCase("", 0);
    runTestCase("()(()", 2);
    runTestCase("((()))())", 8);
    return 0;
}