#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
    int* items;
    int top;
    unsigned int capacity;
} Stack;

void destroyStack(Stack* stack); // Forward declaration

Stack* createStack(unsigned int capacity) {
    if (capacity == 0) {
        capacity = 4; // Default initial capacity
    }
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    if (!stack) {
        perror("Failed to allocate memory for stack structure");
        return NULL;
    }
    stack->capacity = capacity;
    stack->top = -1;
    stack->items = (int*)malloc(stack->capacity * sizeof(int));
    if (!stack->items) {
        perror("Failed to allocate memory for stack items");
        free(stack);
        return NULL;
    }
    return stack;
}

void destroyStack(Stack* stack) {
    if (stack) {
        if (stack->items) {
            free(stack->items);
        }
        free(stack);
    }
}

int isEmpty(Stack* stack) {
    if (!stack) return 1;
    return stack->top == -1;
}

int isFull(Stack* stack) {
    if (!stack) return 1;
    return stack->top == (int)stack->capacity - 1;
}

int resizeStack(Stack* stack) {
    if (!stack) return 0;
    unsigned int newCapacity = stack->capacity * 2;
    if (newCapacity <= stack->capacity) { // Overflow check
        fprintf(stderr, "Error: New capacity would overflow.\n");
        return 0;
    }
    int* newItems = (int*)realloc(stack->items, newCapacity * sizeof(int));
    if (!newItems) {
        perror("Failed to reallocate memory for stack");
        return 0;
    }
    stack->items = newItems;
    stack->capacity = newCapacity;
    return 1;
}

void push(Stack* stack, int item) {
    if (!stack) return;
    if (isFull(stack)) {
        if (!resizeStack(stack)) {
            fprintf(stderr, "Error: Stack overflow and resize failed.\n");
            destroyStack(stack);
            exit(EXIT_FAILURE);
        }
    }
    stack->items[++stack->top] = item;
}

int pop(Stack* stack) {
    if (isEmpty(stack)) {
        fprintf(stderr, "Error: Stack underflow on pop.\n");
        destroyStack(stack);
        exit(EXIT_FAILURE);
    }
    return stack->items[stack->top--];
}

int peek(Stack* stack) {
    if (isEmpty(stack)) {
        fprintf(stderr, "Error: Stack underflow on peek.\n");
        destroyStack(stack);
        exit(EXIT_FAILURE);
    }
    return stack->items[stack->top];
}

int main() {
    // Test Case 1: Push and Peek
    printf("--- Test Case 1: Push and Peek ---\n");
    Stack* stack1 = createStack(4);
    if (!stack1) return EXIT_FAILURE;
    push(stack1, 10);
    push(stack1, 20);
    push(stack1, 30);
    printf("Pushed 10, 20, 30. Top element (peek): %d\n", peek(stack1));
    printf("Stack size: %d\n", stack1->top + 1);
    destroyStack(stack1);

    // Test Case 2: Push and Pop
    printf("\n--- Test Case 2: Push and Pop ---\n");
    Stack* stack2 = createStack(4);
    if (!stack2) return EXIT_FAILURE;
    push(stack2, 10);
    push(stack2, 20);
    push(stack2, 30);
    printf("Popped element: %d\n", pop(stack2));
    printf("Top element after pop (peek): %d\n", peek(stack2));
    printf("Stack size: %d\n", stack2->top + 1);
    destroyStack(stack2);

    // Test Case 3: Mixed Operations
    printf("\n--- Test Case 3: Mixed Operations ---\n");
    Stack* stack3 = createStack(4);
    if (!stack3) return EXIT_FAILURE;
    push(stack3, 10);
    push(stack3, 20);
    printf("Popped element: %d\n", pop(stack3));
    push(stack3, 30);
    printf("Top element (peek): %d\n", peek(stack3));
    printf("Stack size: %d\n", stack3->top + 1);
    destroyStack(stack3);

    // Test Case 4: Dynamic resizing
    printf("\n--- Test Case 4: Dynamic resizing ---\n");
    Stack* stack4 = createStack(2);
    if (!stack4) return EXIT_FAILURE;
    printf("Initial capacity: %u\n", stack4->capacity);
    push(stack4, 10);
    push(stack4, 20);
    printf("Stack full. Pushing another item...\n");
    push(stack4, 30);
    printf("New capacity: %u\n", stack4->capacity);
    printf("Top element (peek): %d\n", peek(stack4));
    printf("Stack size: %d\n", stack4->top + 1);
    destroyStack(stack4);

    // Test Case 5: Pop all elements
    printf("\n--- Test Case 5: Pop all elements ---\n");
    Stack* stack5 = createStack(3);
    if (!stack5) return EXIT_FAILURE;
    push(stack5, 1);
    push(stack5, 2);
    push(stack5, 3);
    printf("Popping: %d\n", pop(stack5));
    printf("Popping: %d\n", pop(stack5));
    printf("Popping: %d\n", pop(stack5));
    printf("Stack is now empty: %s\n", isEmpty(stack5) ? "Yes" : "No");
    // Note: Calling pop() or peek() now would exit the program, demonstrating underflow handling.
    destroyStack(stack5);

    printf("\nAll tests completed successfully.\n");
    return 0;
}