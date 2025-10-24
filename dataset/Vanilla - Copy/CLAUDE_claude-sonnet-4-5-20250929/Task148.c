
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct Stack {
    Node* top;
    int size;
} Stack;

Stack* createStack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->top = NULL;
    stack->size = 0;
    return stack;
}

void push(Stack* stack, int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = stack->top;
    stack->top = newNode;
    stack->size++;
}

int pop(Stack* stack) {
    if (stack->top == NULL) {
        fprintf(stderr, "Pop from empty stack\\n");
        exit(1);
    }
    int data = stack->top->data;
    Node* temp = stack->top;
    stack->top = stack->top->next;
    free(temp);
    stack->size--;
    return data;
}

int peek(Stack* stack) {
    if (stack->top == NULL) {
        fprintf(stderr, "Peek from empty stack\\n");
        exit(1);
    }
    return stack->top->data;
}

bool isEmpty(Stack* stack) {
    return stack->top == NULL;
}

int getSize(Stack* stack) {
    return stack->size;
}

void freeStack(Stack* stack) {
    while (!isEmpty(stack)) {
        pop(stack);
    }
    free(stack);
}

int main() {
    // Test Case 1: Basic push and peek
    printf("Test Case 1: Basic push and peek\\n");
    Stack* stack1 = createStack();
    push(stack1, 10);
    push(stack1, 20);
    push(stack1, 30);
    printf("Peek: %d\\n", peek(stack1));
    printf("Size: %d\\n", getSize(stack1));
    printf("\\n");
    freeStack(stack1);
    
    // Test Case 2: Pop operations
    printf("Test Case 2: Pop operations\\n");
    Stack* stack2 = createStack();
    push(stack2, 5);
    push(stack2, 15);
    push(stack2, 25);
    printf("Pop: %d\\n", pop(stack2));
    printf("Pop: %d\\n", pop(stack2));
    printf("Peek: %d\\n", peek(stack2));
    printf("Size: %d\\n", getSize(stack2));
    printf("\\n");
    freeStack(stack2);
    
    // Test Case 3: Push and pop multiple elements
    printf("Test Case 3: Push and pop multiple elements\\n");
    Stack* stack3 = createStack();
    for (int i = 1; i <= 5; i++) {
        push(stack3, i * 10);
    }
    printf("Peek: %d\\n", peek(stack3));
    while (!isEmpty(stack3)) {
        printf("Pop: %d\\n", pop(stack3));
    }
    printf("Is Empty: %s\\n", isEmpty(stack3) ? "true" : "false");
    printf("\\n");
    freeStack(stack3);
    
    // Test Case 4: Single element operations
    printf("Test Case 4: Single element operations\\n");
    Stack* stack4 = createStack();
    push(stack4, 100);
    printf("Peek: %d\\n", peek(stack4));
    printf("Pop: %d\\n", pop(stack4));
    printf("Is Empty: %s\\n", isEmpty(stack4) ? "true" : "false");
    printf("\\n");
    freeStack(stack4);
    
    // Test Case 5: Mixed operations
    printf("Test Case 5: Mixed operations\\n");
    Stack* stack5 = createStack();
    push(stack5, 7);
    push(stack5, 14);
    printf("Peek: %d\\n", peek(stack5));
    push(stack5, 21);
    printf("Pop: %d\\n", pop(stack5));
    push(stack5, 28);
    printf("Peek: %d\\n", peek(stack5));
    printf("Size: %d\\n", getSize(stack5));
    freeStack(stack5);
    
    return 0;
}
