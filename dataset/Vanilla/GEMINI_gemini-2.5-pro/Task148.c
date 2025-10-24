#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// A structure to represent a stack
struct Stack {
    int top;
    unsigned capacity;
    int* array;
};

// Function to create a stack of given capacity.
struct Stack* createStack(unsigned capacity) {
    struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack));
    stack->capacity = capacity;
    stack->top = -1;
    stack->array = (int*)malloc(stack->capacity * sizeof(int));
    if (!stack->array) {
        // Handle memory allocation failure
        return NULL;
    }
    return stack;
}

// Function to free the stack memory
void freeStack(struct Stack* stack) {
    if (stack) {
        free(stack->array);
        free(stack);
    }
}

// Stack is full when top is equal to the last index
int isFull(struct Stack* stack) {
    return stack->top == stack->capacity - 1;
}

// Stack is empty when top is -1
int isEmpty(struct Stack* stack) {
    return stack->top == -1;
}

// Function to add an item to stack. It increases top by 1
void push(struct Stack* stack, int item) {
    if (isFull(stack)) {
        printf("Stack Overflow\n");
        return;
    }
    printf("Pushing %d\n", item);
    stack->array[++stack->top] = item;
}

// Function to remove an item from stack. It decreases top by 1
int pop(struct Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack Underflow\n");
        return INT_MIN;
    }
    int item = stack->array[stack->top--];
    printf("Popping %d\n", item);
    return item;
}

// Function to return the top from stack without removing it
int peek(struct Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack is empty\n");
        return INT_MIN;
    }
    return stack->array[stack->top];
}

int main() {
    printf("--- C Stack Test ---\n");
    struct Stack* stack = createStack(3);
    if (!stack) {
        printf("Failed to create stack\n");
        return 1;
    }

    // Test Case 1: Push elements
    push(stack, 10);
    push(stack, 20);
    push(stack, 30);

    // Test Case 2: Peek at the top element
    printf("Top element is: %d\n", peek(stack));

    // Test Case 3: Pop an element
    pop(stack);
    printf("Top element after pop is: %d\n", peek(stack));

    // Test Case 4: Push to a full stack (Overflow)
    push(stack, 40);
    push(stack, 50);

    // Test Case 5: Pop all elements and check for underflow
    pop(stack);
    pop(stack);
    pop(stack);
    pop(stack);

    freeStack(stack);
    return 0;
}