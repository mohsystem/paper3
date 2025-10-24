#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/**
 * A structure to represent a stack.
 * This implementation uses a fixed-size array and handles memory management,
 * null pointers, stack overflow, and stack underflow.
 */
typedef struct {
    int top;
    unsigned capacity;
    int* array;
} Stack;

// Function to create a stack of a given capacity.
Stack* createStack(unsigned capacity) {
    // Secure: Prevent creating a zero-capacity stack.
    if (capacity == 0) {
        fprintf(stderr, "Error: Stack capacity must be greater than 0.\n");
        return NULL;
    }
    
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    // Secure: Check for memory allocation failure.
    if (!stack) {
        perror("Failed to allocate memory for stack structure");
        return NULL;
    }

    stack->capacity = capacity;
    stack->top = -1;
    stack->array = (int*)malloc(stack->capacity * sizeof(int));

    // Secure: Check for memory allocation failure.
    if (!stack->array) {
        perror("Failed to allocate memory for stack array");
        free(stack); // Clean up partially allocated structure.
        return NULL;
    }
    return stack;
}

// Function to free the memory used by the stack.
void freeStack(Stack* stack) {
    if (stack) {
        free(stack->array);
        free(stack);
    }
}

// Returns 1 if stack is full, 0 otherwise.
int isFull(Stack* stack) {
    // Secure: Null pointer check.
    if (!stack) return 0;
    return stack->top == (int)stack->capacity - 1;
}

// Returns 1 if stack is empty, 0 otherwise.
int isEmpty(Stack* stack) {
    // Secure: Null pointer check.
    if (!stack) return 1; // Treat null stack as empty.
    return stack->top == -1;
}

// Pushes an item to the stack.
void push(Stack* stack, int item) {
    // Secure: Null pointer check.
    if (!stack) {
        fprintf(stderr, "Error: Cannot push to a NULL stack.\n");
        return;
    }
    // Secure: Check for stack overflow.
    if (isFull(stack)) {
        fprintf(stderr, "Error: Stack overflow on push(%d).\n", item);
        return;
    }
    stack->array[++stack->top] = item;
    printf("%d pushed to stack\n", item);
}

// Pops and returns an item from the stack.
int pop(Stack* stack) {
    // Secure: Null pointer check.
    if (!stack) {
        fprintf(stderr, "Error: Cannot pop from a NULL stack.\n");
        return INT_MIN; // Error value
    }
    // Secure: Check for stack underflow.
    if (isEmpty(stack)) {
        fprintf(stderr, "Error: Stack underflow on pop.\n");
        return INT_MIN; // Error value
    }
    return stack->array[stack->top--];
}

// Returns the top item from the stack without removing it.
int peek(Stack* stack) {
    // Secure: Null pointer check.
    if (!stack) {
        fprintf(stderr, "Error: Cannot peek from a NULL stack.\n");
        return INT_MIN; // Error value
    }
    // Secure: Check for stack underflow.
    if (isEmpty(stack)) {
        fprintf(stderr, "Error: Stack underflow on peek.\n");
        return INT_MIN; // Error value
    }
    return stack->array[stack->top];
}

// Main function with test cases.
int main() {
    printf("C Stack Test Cases:\n");

    // Test Case 1: Basic push and pop
    printf("\n--- Test Case 1: Basic Operations ---\n");
    Stack* s1 = createStack(100);
    if (!s1) return 1; // Exit if creation failed
    push(s1, 10);
    push(s1, 20);
    printf("Popped: %d\n", pop(s1));
    printf("Peeked: %d\n", peek(s1));
    printf("Popped: %d\n", pop(s1));
    freeStack(s1);

    // Test Case 2: Underflow on pop and peek
    printf("\n--- Test Case 2: Underflow Operations ---\n");
    Stack* s2 = createStack(100);
    if (!s2) return 1;
    printf("Attempting to pop from empty stack...\n");
    pop(s2);
    printf("Attempting to peek from empty stack...\n");
    peek(s2);
    freeStack(s2);

    // Test Case 3: Stack overflow
    printf("\n--- Test Case 3: Stack Overflow ---\n");
    Stack* s3 = createStack(3); // Small capacity for testing
    if (!s3) return 1;
    push(s3, 1);
    push(s3, 2);
    push(s3, 3);
    printf("Attempting to push to a full stack...\n");
    push(s3, 4); // This should cause an overflow error
    freeStack(s3);

    // Test Case 4: Sequence of operations
    printf("\n--- Test Case 4: Sequence of Operations ---\n");
    Stack* s4 = createStack(100);
    if (!s4) return 1;
    push(s4, 1);
    push(s4, 2);
    push(s4, 3);
    printf("Popped: %d\n", pop(s4)); // 3
    push(s4, 4);
    printf("Peeked: %d\n", peek(s4)); // 4
    printf("Popped: %d\n", pop(s4)); // 4
    freeStack(s4);

    // Test Case 5: isEmpty Check
    printf("\n--- Test Case 5: isEmpty Check ---\n");
    Stack* s5 = createStack(100);
    if (!s5) return 1;
    printf("Is stack empty? %s\n", isEmpty(s5) ? "true" : "false");
    push(s5, 100);
    printf("Is stack empty? %s\n", isEmpty(s5) ? "true" : "false");
    pop(s5);
    printf("Is stack empty? %s\n", isEmpty(s5) ? "true" : "false");
    freeStack(s5);
    
    return 0;
}