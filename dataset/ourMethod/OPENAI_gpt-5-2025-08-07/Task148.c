#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    int *data;
    size_t capacity;
    size_t top;
} IntStack;

typedef struct {
    int ok;   /* 1 if value is valid, 0 otherwise */
    int value;
} Result;

int init_stack(IntStack *s, size_t capacity) {
    if (s == NULL || capacity == 0 || capacity > 1000000u) {
        return 0;
    }
    s->data = (int *)malloc(sizeof(int) * capacity);
    if (s->data == NULL) {
        s->capacity = 0;
        s->top = 0;
        return 0;
    }
    s->capacity = capacity;
    s->top = 0;
    return 1;
}

void free_stack(IntStack *s) {
    if (s != NULL) {
        free(s->data);
        s->data = NULL;
        s->capacity = 0;
        s->top = 0;
    }
}

int push_stack(IntStack *s, int value) {
    if (s == NULL || s->data == NULL) {
        return 0;
    }
    if (s->top >= s->capacity) {
        return 0;
    }
    s->data[s->top++] = value;
    return 1;
}

Result pop_stack(IntStack *s) {
    Result r;
    r.ok = 0;
    r.value = 0;
    if (s == NULL || s->data == NULL || s->top == 0) {
        return r;
    }
    s->top--;
    r.ok = 1;
    r.value = s->data[s->top];
    return r;
}

Result peek_stack(const IntStack *s) {
    Result r;
    r.ok = 0;
    r.value = 0;
    if (s == NULL || s->data == NULL || s->top == 0) {
        return r;
    }
    r.ok = 1;
    r.value = s->data[s->top - 1];
    return r;
}

size_t size_stack(const IntStack *s) {
    if (s == NULL) {
        return 0;
    }
    return s->top;
}

int is_empty_stack(const IntStack *s) {
    if (s == NULL) {
        return 1;
    }
    return s->top == 0 ? 1 : 0;
}

int main(void) {
    // Test 1: Basic push/pop/peek
    IntStack s1;
    if (!init_stack(&s1, 3)) {
        printf("Failed to init s1\n");
        return 1;
    }
    printf("T1 push 10: %s\n", push_stack(&s1, 10) ? "true" : "false");
    printf("T1 push 20: %s\n", push_stack(&s1, 20) ? "true" : "false");
    Result r = peek_stack(&s1);
    if (r.ok) printf("T1 peek: %d\n", r.value); else printf("T1 peek: None\n");
    r = pop_stack(&s1);
    if (r.ok) printf("T1 pop: %d\n", r.value); else printf("T1 pop: None\n");
    r = pop_stack(&s1);
    if (r.ok) printf("T1 pop: %d\n", r.value); else printf("T1 pop: None\n");
    r = pop_stack(&s1);
    if (r.ok) printf("T1 pop empty: %d\n", r.value); else printf("T1 pop empty: None\n");
    free_stack(&s1);

    // Test 2: Overflow behavior
    IntStack s2;
    if (!init_stack(&s2, 1)) {
        printf("Failed to init s2\n");
        return 1;
    }
    printf("T2 push 5: %s\n", push_stack(&s2, 5) ? "true" : "false");
    printf("T2 push 6 overflow: %s\n", push_stack(&s2, 6) ? "true" : "false");
    r = peek_stack(&s2);
    if (r.ok) printf("T2 peek: %d\n", r.value); else printf("T2 peek: None\n");
    printf("T2 size: %zu\n", size_stack(&s2));
    free_stack(&s2);

    // Test 3: Empty stack operations
    IntStack s3;
    if (!init_stack(&s3, 2)) {
        printf("Failed to init s3\n");
        return 1;
    }
    r = peek_stack(&s3);
    if (r.ok) printf("T3 empty peek: %d\n", r.value); else printf("T3 empty peek: None\n");
    r = pop_stack(&s3);
    if (r.ok) printf("T3 empty pop: %d\n", r.value); else printf("T3 empty pop: None\n");
    printf("T3 isEmpty: %s\n", is_empty_stack(&s3) ? "true" : "false");
    free_stack(&s3);

    // Test 4: Fill and partial pop
    IntStack s4;
    if (!init_stack(&s4, 4)) {
        printf("Failed to init s4\n");
        return 1;
    }
    printf("T4 push 1: %s\n", push_stack(&s4, 1) ? "true" : "false");
    printf("T4 push 2: %s\n", push_stack(&s4, 2) ? "true" : "false");
    printf("T4 push 3: %s\n", push_stack(&s4, 3) ? "true" : "false");
    printf("T4 size: %zu\n", size_stack(&s4));
    r = pop_stack(&s4);
    if (r.ok) printf("T4 pop: %d\n", r.value); else printf("T4 pop: None\n");
    r = peek_stack(&s4);
    if (r.ok) printf("T4 peek: %d\n", r.value); else printf("T4 peek: None\n");
    free_stack(&s4);

    // Test 5: Mixed operations
    IntStack s5;
    if (!init_stack(&s5, 5)) {
        printf("Failed to init s5\n");
        return 1;
    }
    printf("T5 push 7: %s\n", push_stack(&s5, 7) ? "true" : "false");
    printf("T5 push 8: %s\n", push_stack(&s5, 8) ? "true" : "false");
    printf("T5 push 9: %s\n", push_stack(&s5, 9) ? "true" : "false");
    r = peek_stack(&s5);
    if (r.ok) printf("T5 peek: %d\n", r.value); else printf("T5 peek: None\n");
    r = pop_stack(&s5);
    if (r.ok) printf("T5 pop: %d\n", r.value); else printf("T5 pop: None\n");
    printf("T5 push 10: %s\n", push_stack(&s5, 10) ? "true" : "false");
    printf("T5 size: %zu\n", size_stack(&s5));
    free_stack(&s5);

    return 0;
}