
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define DEFAULT_CAPACITY 10

typedef struct {
    int* stackArray;
    int top;
    int capacity;
} Task148;

Task148* createStack(int capacity) {
    if (capacity <= 0) {
        fprintf(stderr, "Error: Capacity must be positive\\n");
        return NULL;
    }
    
    Task148* stack = (Task148*)malloc(sizeof(Task148));
    if (stack == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    stack->stackArray = (int*)malloc(capacity * sizeof(int));
    if (stack->stackArray == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        free(stack);
        return NULL;
    }
    
    stack->capacity = capacity;
    stack->top = -1;
    return stack;
}

void destroyStack(Task148* stack) {
    if (stack != NULL) {
        if (stack->stackArray != NULL) {
            free(stack->stackArray);
        }
        free(stack);
    }
}

bool isEmpty(Task148* stack) {
    return stack != NULL && stack->top == -1;
}

bool isFull(Task148* stack) {
    return stack != NULL && stack->top == stack->capacity - 1;
}

int size(Task148* stack) {
    return stack != NULL ? stack->top + 1 : 0;
}

bool resize(Task148* stack) {
    if (stack == NULL) return false;
    
    int newCapacity = stack->capacity * 2;
    int* newArray = (int*)realloc(stack->stackArray, newCapacity * sizeof(int));
    
    if (newArray == NULL) {
        fprintf(stderr, "Error: Memory reallocation failed\\n");
        return false;
    }
    
    stack->stackArray = newArray;
    stack->capacity = newCapacity;
    return true;
}

bool push(Task148* stack, int value) {
    if (stack == NULL) {
        fprintf(stderr, "Error: Stack is NULL\\n");
        return false;
    }
    
    if (isFull(stack)) {
        if (!resize(stack)) {
            return false;
        }
    }
    
    stack->stackArray[++stack->top] = value;
    return true;
}

bool pop(Task148* stack, int* value) {
    if (stack == NULL) {
        fprintf(stderr, "Error: Stack is NULL\\n");
        return false;
    }
    
    if (isEmpty(stack)) {
        fprintf(stderr, "Error: Cannot pop from empty stack\\n");
        return false;
    }
    
    if (value != NULL) {
        *value = stack->stackArray[stack->top--];
    }
    return true;
}

bool peek(Task148* stack, int* value) {
    if (stack == NULL) {
        fprintf(stderr, "Error: Stack is NULL\\n");
        return false;
    }
    
    if (isEmpty(stack)) {
        fprintf(stderr, "Error: Cannot peek empty stack\\n");
        return false;
    }
    
    if (value != NULL) {
        *value = stack->stackArray[stack->top];
    }
    return true;
}

int main() {
    int value;
    
    printf("=== Test Case 1: Basic Push and Pop ===\\n");
    Task148* stack1 = createStack(DEFAULT_CAPACITY);
    push(stack1, 10);
    push(stack1, 20);
    push(stack1, 30);
    if (peek(stack1, &value)) printf("Peek: %d\\n", value);
    if (pop(stack1, &value)) printf("Pop: %d\\n", value);
    if (pop(stack1, &value)) printf("Pop: %d\\n", value);
    printf("Size: %d\\n", size(stack1));
    destroyStack(stack1);

    printf("\\n=== Test Case 2: Empty Stack Check ===\\n");
    Task148* stack2 = createStack(DEFAULT_CAPACITY);
    printf("Is Empty: %s\\n", isEmpty(stack2) ? "true" : "false");
    push(stack2, 5);
    printf("Is Empty: %s\\n", isEmpty(stack2) ? "true" : "false");
    pop(stack2, &value);
    printf("Is Empty after pop: %s\\n", isEmpty(stack2) ? "true" : "false");
    destroyStack(stack2);

    printf("\\n=== Test Case 3: Multiple Operations ===\\n");
    Task148* stack3 = createStack(DEFAULT_CAPACITY);
    for (int i = 1; i <= 5; i++) {
        push(stack3, i * 10);
    }
    printf("Stack size: %d\\n", size(stack3));
    if (peek(stack3, &value)) printf("Peek: %d\\n", value);
    while (!isEmpty(stack3)) {
        if (pop(stack3, &value)) printf("Pop: %d\\n", value);
    }
    destroyStack(stack3);

    printf("\\n=== Test Case 4: Stack Resize ===\\n");
    Task148* stack4 = createStack(2);
    push(stack4, 1);
    push(stack4, 2);
    push(stack4, 3);
    push(stack4, 4);
    printf("Size after resize: %d\\n", size(stack4));
    if (peek(stack4, &value)) printf("Peek: %d\\n", value);
    destroyStack(stack4);

    printf("\\n=== Test Case 5: Error Handling ===\\n");
    Task148* stack5 = createStack(DEFAULT_CAPACITY);
    printf("Attempting to pop from empty stack:\\n");
    pop(stack5, &value);
    printf("Attempting to peek empty stack:\\n");
    peek(stack5, &value);
    destroyStack(stack5);

    return 0;
}
