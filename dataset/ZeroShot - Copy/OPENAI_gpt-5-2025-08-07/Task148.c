#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} Stack;

typedef struct {
    int value;
    int ok; /* 1 if valid, 0 if underflow */
} StackResult;

static int ensure_capacity(Stack *s, size_t minCapacity) {
    if (!s) return 0;
    if (s->capacity >= minCapacity) return 1;

    size_t newCap = s->capacity ? s->capacity : 16u;
    while (newCap < minCapacity) {
        if (newCap > SIZE_MAX / 2) { /* prevent overflow on doubling */
            newCap = SIZE_MAX;
            break;
        }
        newCap *= 2;
    }

    if (newCap < minCapacity) return 0;
    if (newCap > SIZE_MAX / sizeof(int)) return 0;

    size_t bytes = newCap * sizeof(int);
    int *newData = (int *)realloc(s->data, bytes);
    if (!newData) return 0;

    /* Optionally zero new memory region for safety */
    if (newCap > s->capacity) {
        size_t oldBytes = s->capacity * sizeof(int);
        memset((unsigned char*)newData + oldBytes, 0, bytes - oldBytes);
    }

    s->data = newData;
    s->capacity = newCap;
    return 1;
}

static void stack_init(Stack *s) {
    if (!s) return;
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
}

static void stack_free(Stack *s) {
    if (!s) return;
    if (s->data) {
        /* Clear memory before free for safety */
        memset(s->data, 0, s->capacity * sizeof(int));
        free(s->data);
    }
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
}

static int stack_push(Stack *s, int value) {
    if (!s) return 0;
    if (!ensure_capacity(s, s->size + 1)) return 0;
    s->data[s->size++] = value;
    return 1;
}

static StackResult stack_pop(Stack *s) {
    StackResult r;
    r.ok = 0;
    r.value = 0;
    if (!s || s->size == 0) {
        return r;
    }
    r.value = s->data[s->size - 1];
    s->data[s->size - 1] = 0; /* clear for safety */
    s->size--;
    r.ok = 1;
    return r;
}

static StackResult stack_peek(const Stack *s) {
    StackResult r;
    r.ok = 0;
    r.value = 0;
    if (!s || s->size == 0) {
        return r;
    }
    r.value = s->data[s->size - 1];
    r.ok = 1;
    return r;
}

/* Test harness with 5 test cases */
int main(void) {
    Stack s;
    stack_init(&s);

    /* Test 1: push 1,2,3 and peek */
    if (stack_push(&s, 1) && stack_push(&s, 2) && stack_push(&s, 3)) {
        StackResult r = stack_peek(&s);
        if (r.ok) {
            printf("Test1 Peek: %d\n", r.value);
        } else {
            printf("Test1 Exception: Stack underflow\n");
        }
    } else {
        printf("Test1 Exception: Memory allocation failure\n");
    }

    /* Test 2: pop -> 3 */
    StackResult r2 = stack_pop(&s);
    if (r2.ok) printf("Test2 Pop: %d\n", r2.value);
    else printf("Test2 Exception: Stack underflow\n");

    /* Test 3: pop -> 2 */
    StackResult r3 = stack_pop(&s);
    if (r3.ok) printf("Test3 Pop: %d\n", r3.value);
    else printf("Test3 Exception: Stack underflow\n");

    /* Test 4: push 4 then pop -> 4 */
    if (stack_push(&s, 4)) {
        StackResult r4 = stack_pop(&s);
        if (r4.ok) printf("Test4 Pop: %d\n", r4.value);
        else printf("Test4 Exception: Stack underflow\n");
    } else {
        printf("Test4 Exception: Memory allocation failure\n");
    }

    /* Test 5: pop remaining 1 then attempt another pop -> underflow */
    StackResult r5a = stack_pop(&s);
    if (r5a.ok) printf("Test5 Pop: %d\n", r5a.value);
    else printf("Test5 Exception: Stack underflow\n");

    StackResult r5b = stack_pop(&s);
    if (r5b.ok) printf("Test5 Pop: %d\n", r5b.value);
    else printf("Test5 Exception: Stack underflow\n");

    stack_free(&s);
    return 0;
}