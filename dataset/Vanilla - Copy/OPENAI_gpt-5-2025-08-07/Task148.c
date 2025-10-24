#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *arr;
    int capacity;
    int size;
} Stack;

void init_stack(Stack *s, int initial_capacity) {
    if (initial_capacity <= 0) initial_capacity = 8;
    s->arr = (int*)malloc(initial_capacity * sizeof(int));
    if (!s->arr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    s->capacity = initial_capacity;
    s->size = 0;
}

static void ensure_capacity(Stack *s) {
    if (s->size >= s->capacity) {
        int new_cap = s->capacity * 2;
        int *new_arr = (int*)realloc(s->arr, new_cap * sizeof(int));
        if (!new_arr) {
            fprintf(stderr, "Memory reallocation failed\n");
            free(s->arr);
            exit(1);
        }
        s->arr = new_arr;
        s->capacity = new_cap;
    }
}

void push(Stack *s, int value) {
    ensure_capacity(s);
    s->arr[s->size++] = value;
}

int pop(Stack *s) {
    if (s->size == 0) {
        fprintf(stderr, "Stack underflow\n");
        exit(1);
    }
    return s->arr[--s->size];
}

int peek(Stack *s) {
    if (s->size == 0) {
        fprintf(stderr, "Stack is empty\n");
        exit(1);
    }
    return s->arr[s->size - 1];
}

int is_empty(Stack *s) {
    return s->size == 0;
}

int stack_size(Stack *s) {
    return s->size;
}

void free_stack(Stack *s) {
    free(s->arr);
    s->arr = NULL;
    s->capacity = 0;
    s->size = 0;
}

int main() {
    Stack s;
    init_stack(&s, 4);
    push(&s, 10);
    push(&s, 20);
    push(&s, 30);
    printf("%d\n", peek(&s)); // Test 1
    printf("%d\n", pop(&s));  // Test 2
    push(&s, 40);
    printf("%d\n", peek(&s)); // Test 3
    printf("%d\n", pop(&s));  // Test 4
    printf("%d\n", pop(&s));  // Test 5
    free_stack(&s);
    return 0;
}