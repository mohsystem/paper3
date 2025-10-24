
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
        printf("Error: Cannot pop from empty stack\\n");
        return -1;
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
        printf("Error: Cannot peek empty stack\\n");
        return -1;
    }
    return stack->top->data;
}

bool isEmpty(Stack* stack) {
    return stack->top == NULL;
}

int getSize(Stack* stack) {
    return stack->size;
}

void destroyStack(Stack* stack) {
    while (!isEmpty(stack)) {
        pop(stack);
    }
    free(stack);
}

int main() {
    // Test Case 1: Basic push and pop operations
    printf("Test Case 1: Basic push and pop\\n");
    Stack* stack1 = createStack();
    push(stack1, 10);
    push(stack1, 20);
    push(stack1, 30);
    printf("Peek: %d\\n", peek(stack1));
    printf("Pop: %d\\n", pop(stack1));
    printf("Pop: %d\\n", pop(stack1));
    printf("Size: %d\\n", getSize(stack1));
    printf("\\n");
    destroyStack(stack1);
    
    // Test Case 2: Push multiple elements and peek
    printf("Test Case 2: Multiple pushes and peek\\n");
    Stack* stack2 = createStack();
    for (int i = 1; i <= 5; i++) {
        push(stack2, i * 10);
    }
    printf("Peek: %d\\n", peek(stack2));
    printf("Size: %d\\n", getSize(stack2));
    printf("\\n");
    destroyStack(stack2);
    
    // Test Case 3: Pop all elements
    printf("Test Case 3: Pop all elements\\n");
    Stack* stack3 = createStack();
    push(stack3, 100);
    push(stack3, 200);
    push(stack3, 300);
    while (!isEmpty(stack3)) {
        printf("Popped: %d\\n", pop(stack3));
    }
    printf("Is empty: %s\\n", isEmpty(stack3) ? "true" : "false");
    printf("\\n");
    destroyStack(stack3);
    
    // Test Case 4: Mixed operations
    printf("Test Case 4: Mixed operations\\n");
    Stack* stack4 = createStack();
    push(stack4, 5);
    push(stack4, 15);
    printf("Peek: %d\\n", peek(stack4));
    pop(stack4);
    push(stack4, 25);
    push(stack4, 35);
    printf("Peek: %d\\n", peek(stack4));
    printf("Size: %d\\n", getSize(stack4));
    printf("\\n");
    destroyStack(stack4);
    
    // Test Case 5: Empty stack exception handling
    printf("Test Case 5: Empty stack exception\\n");
    Stack* stack5 = createStack();
    pop(stack5);
    peek(stack5);
    destroyStack(stack5);
    
    return 0;
}
