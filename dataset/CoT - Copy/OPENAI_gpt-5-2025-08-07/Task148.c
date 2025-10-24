#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} Stack;

typedef struct {
    int ok;
    int value;
} IntResult;

static Stack* create_stack(void) {
    Stack *s = (Stack*)calloc(1, sizeof(Stack));
    if (!s) return NULL;
    s->capacity = 16;
    s->data = (int*)calloc(s->capacity, sizeof(int));
    if (!s->data) {
        free(s);
        return NULL;
    }
    s->size = 0;
    return s;
}

static void destroy_stack(Stack *s) {
    if (!s) return;
    free(s->data);
    s->data = NULL;
    s->capacity = 0;
    s->size = 0;
    free(s);
}

static int ensure_capacity(Stack *s) {
    if (s->size < s->capacity) return 1;
    size_t newCap = s->capacity > (SIZE_MAX / 2) ? SIZE_MAX : s->capacity * 2;
    if (newCap <= s->capacity) return 0;
    if (newCap > SIZE_MAX / sizeof(int)) return 0;
    int *newData = (int*)realloc(s->data, newCap * sizeof(int));
    if (!newData) return 0;
    s->data = newData;
    s->capacity = newCap;
    return 1;
}

static int push_stack(Stack *s, int value) {
    if (!s) return 0;
    if (!ensure_capacity(s)) return 0;
    s->data[s->size++] = value;
    return 1;
}

static IntResult pop_stack(Stack *s) {
    IntResult r;
    r.ok = 0;
    r.value = 0;
    if (!s || s->size == 0) return r;
    r.ok = 1;
    r.value = s->data[--s->size];
    s->data[s->size] = 0;
    return r;
}

static IntResult peek_stack(const Stack *s) {
    IntResult r;
    r.ok = 0;
    r.value = 0;
    if (!s || s->size == 0) return r;
    r.ok = 1;
    r.value = s->data[s->size - 1];
    return r;
}

static int is_empty(const Stack *s) {
    return (!s || s->size == 0) ? 1 : 0;
}

static size_t stack_size(const Stack *s) {
    return s ? s->size : 0;
}

int main(void) {
    Stack *s = create_stack();
    if (!s) {
        printf("Failed to create stack\n");
        return 1;
    }

    // Test 1: Push 1,2,3 then peek
    int t1a = push_stack(s, 1);
    int t1b = push_stack(s, 2);
    int t1c = push_stack(s, 3);
    IntResult t1p = peek_stack(s);
    printf("Test1: pushes=%d,%d,%d peekOk=%d peekVal=%d\n", t1a, t1b, t1c, t1p.ok, t1p.ok ? t1p.value : 0);

    // Test 2: Pop once
    IntResult t2 = pop_stack(s);
    printf("Test2: popOk=%d val=%d\n", t2.ok, t2.ok ? t2.value : 0);

    // Test 3: Peek after pop
    IntResult t3 = peek_stack(s);
    printf("Test3: peekOk=%d val=%d\n", t3.ok, t3.ok ? t3.value : 0);

    // Test 4: Pop remaining and attempt extra pop
    IntResult t4a = pop_stack(s);
    IntResult t4b = pop_stack(s);
    IntResult t4c = pop_stack(s); // should fail
    printf("Test4: popVals=%d,%d extraPopOk=%d\n",
           t4a.ok ? t4a.value : 0, t4b.ok ? t4b.value : 0, t4c.ok);

    // Test 5: Push after empty and peek
    int t5a = push_stack(s, 42);
    IntResult t5b = peek_stack(s);
    printf("Test5: pushOk=%d peekOk=%d val=%d\n", t5a, t5b.ok, t5b.ok ? t5b.value : 0);

    destroy_stack(s);
    return 0;
}