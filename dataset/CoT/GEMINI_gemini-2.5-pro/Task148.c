#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // For INT_MIN

// A structure to represent a stack
typedef struct Stack {
    int top;
    unsigned capacity;
    int* array;
} Stack;

// Function to create a stack of given capacity.
Stack* createStack(unsigned capacity) {
    if (capacity == 0) {
        fprintf(stderr, "Error: Stack capacity must be positive.\n");
        return NULL;
    }
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    if (!stack) {
        fprintf(stderr, "Error: Memory allocation failed for stack struct.\n");
        return NULL;
    }
    stack->capacity = capacity;
    stack->top = -1; // Stack is initially empty
    stack->array = (int*)malloc(stack->capacity * sizeof(int));
    if (!stack->array) {
        fprintf(stderr, "Error: Memory allocation failed for stack array.\n");
        free(stack);
        return NULL;
    }
    return stack;
}

// Function to free the stack memory
void freeStack(Stack* stack) {
    if (stack) {
        free(stack->array);
        free(stack);
    }
}

// Stack is full when top is equal to the last index
int isFull(Stack* stack) {
    if (!stack) return 0;
    return stack->top == (int)stack->capacity - 1;
}

// Stack is empty when top is -1
int isEmpty(Stack* stack) {
    if (!stack) return 1;
    return stack->top == -1;
}

// Function to add an item to stack. It increases top by 1
void push(Stack* stack, int item) {
    if (!stack) {
        fprintf(stderr, "Error: Stack is NULL.\n");
        return;
    }
    if (isFull(stack)) {
        fprintf(stderr, "Error: Stack overflow. Cannot push %d\n", item);
        return;
    }
    stack->array[++stack->top] = item;
    printf("Pushed %d to stack.\n", item);
}

// Function to remove an item from stack. It decreases top by 1
int pop(Stack* stack) {
    if (!stack) {
        fprintf(stderr, "Error: Stack is NULL.\n");
        return INT_MIN;
    }
    if (isEmpty(stack)) {
        fprintf(stderr, "Error: Stack underflow. Cannot pop.\n");
        return INT_MIN; // Return a sentinel value for error
    }
    int item = stack->array[stack->top--];
    printf("Popped %d from stack.\n", item);
    return item;
}

// Function to return the top from stack without removing it
int peek(Stack* stack) {
    if (!stack) {
        fprintf(stderr, "Error: Stack is NULL.\n");
        return INT_MIN;
    }
    if (isEmpty(stack)) {
        fprintf(stderr, "Error: Stack is empty. Cannot peek.\n");
        return INT_MIN; // Return a sentinel value for error
    }
    return stack->array[stack->top];
}

int main() {
    printf("Initializing a stack with capacity 5.\n");
    Stack* stack = createStack(5);
    if (!stack) {
        return 1; // Exit if stack creation failed
    }

    // Test Case 1: Push items and peek
    printf("\n--- Test Case 1: Push and Peek ---\n");
    push(stack, 10);
    push(stack, 20);
    push(stack, 30);
    printf("Top element is (peek): %d\n", peek(stack));
    
    // Test Case 2: Pop all items
    printf("\n--- Test Case 2: Pop All Items ---\n");
    pop(stack);
    pop(stack);
    pop(stack);

    // Test Case 3: Pop from an empty stack (underflow)
    printf("\n--- Test Case 3: Pop from Empty Stack ---\n");
    printf("Is stack empty? %s\n", isEmpty(stack) ? "Yes" : "No");
    pop(stack); // This should cause an underflow error

    // Test Case 4: Push until the stack is full
    printf("\n--- Test Case 4: Fill the Stack ---\n");
    push(stack, 11);
    push(stack, 22);
    push(stack, 33);
    push(stack, 44);
    push(stack, 55);
    printf("Is stack full? %s\n", isFull(stack) ? "Yes" : "No");

    // Test Case 5: Push to a full stack (overflow)
    printf("\n--- Test Case 5: Push to Full Stack ---\n");
    push(stack, 66); // This should cause an overflow error
    printf("Top element is (peek): %d\n", peek(stack));

    // Clean up
    freeStack(stack);
    return 0;
}