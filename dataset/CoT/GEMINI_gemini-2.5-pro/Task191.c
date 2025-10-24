#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Stack Data Structure Implementation ---
typedef struct {
    int* items;
    int top;
    unsigned capacity;
} Stack;

// Creates and initializes a stack. Returns NULL on allocation failure.
Stack* createStack(unsigned capacity) {
    Stack* stack = (Stack*) malloc(sizeof(Stack));
    if (!stack) return NULL;
    stack->capacity = capacity;
    stack->top = -1;
    stack->items = (int*) malloc(stack->capacity * sizeof(int));
    if (!stack->items) {
        free(stack);
        return NULL;
    }
    return stack;
}

// Frees the memory allocated for the stack.
void destroyStack(Stack* stack) {
    if (stack) {
        free(stack->items);
        free(stack);
    }
}

// Checks if the stack is empty.
int isEmpty(Stack* stack) {
    return stack->top == -1;
}

// Pushes an item onto the stack. Assumes stack will not overflow.
void push(Stack* stack, int item) {
    stack->items[++stack->top] = item;
}

// Pops an item from the stack. Does nothing if stack is empty.
void pop(Stack* stack) {
    if (!isEmpty(stack)) {
        stack->top--;
    }
}

// Returns the top item of the stack without removing it.
int peek(Stack* stack) {
    if (!isEmpty(stack)) {
        return stack->items[stack->top];
    }
    return -1; // Should not be reached in this problem's logic
}
// --- End Stack Implementation ---

// Helper function to find the maximum of two integers.
int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * @brief Given a string containing just the characters '(' and ')', returns the length of the longest 
 * valid (well-formed) parentheses substring.
 *
 * This function uses a manually implemented stack to keep track of the indices of the parentheses.
 * The logic is identical to the implementations in other languages.
 * 
 * @param s The input string containing only '(' and ')' characters.
 * @return The length of the longest valid parentheses substring.
 */
int longestValidParentheses(const char* s) {
    int n = strlen(s);
    if (n < 2) {
        return 0;
    }

    int maxLength = 0;
    // Capacity n+1 to hold all indices and the initial -1
    Stack* stack = createStack(n + 1);
    if (!stack) {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for stack.\n");
        return 0; 
    }

    push(stack, -1); // Base for the first valid substring

    for (int i = 0; i < n; i++) {
        if (s[i] == '(') {
            push(stack, i);
        } else { // s[i] == ')'
            pop(stack);
            if (isEmpty(stack)) {
                // This ')' is unmatched, it becomes the new base
                push(stack, i);
            } else {
                // A match is found. Calculate the length.
                maxLength = max(maxLength, i - peek(stack));
            }
        }
    }

    destroyStack(stack);
    return maxLength;
}

int main() {
    // Test Case 1
    const char* s1 = "(()";
    printf("Input: \"%s\", Output: %d\n", s1, longestValidParentheses(s1));

    // Test Case 2
    const char* s2 = ")()())";
    printf("Input: \"%s\", Output: %d\n", s2, longestValidParentheses(s2));

    // Test Case 3
    const char* s3 = "";
    printf("Input: \"%s\", Output: %d\n", s3, longestValidParentheses(s3));

    // Test Case 4
    const char* s4 = "()(()";
    printf("Input: \"%s\", Output: %d\n", s4, longestValidParentheses(s4));

    // Test Case 5
    const char* s5 = "((()))";
    printf("Input: \"%s\", Output: %d\n", s5, longestValidParentheses(s5));

    return 0;
}