#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CAPACITY 1000000

typedef struct {
    int success;
    int value;
} DequeueResult;

typedef struct {
    int *buffer;
    int capacity;
    int head;
    int tail;
    int size;
} IntQueue;

int intq_init(IntQueue *q, int capacity) {
    if (q == NULL) return 0;
    if (capacity < 1 || capacity > MAX_CAPACITY) return 0;
    q->buffer = (int *)calloc((size_t)capacity, sizeof(int));
    if (q->buffer == NULL) {
        return 0;
    }
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    return 1;
}

void intq_free(IntQueue *q) {
    if (q == NULL) return;
    if (q->buffer != NULL) {
        free(q->buffer);
        q->buffer = NULL;
    }
    q->capacity = 0;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
}

int intq_size(const IntQueue *q) {
    if (q == NULL) return 0;
    return q->size;
}

int intq_capacity(const IntQueue *q) {
    if (q == NULL) return 0;
    return q->capacity;
}

int intq_is_empty(const IntQueue *q) {
    if (q == NULL) return 1;
    return q->size == 0;
}

int intq_is_full(const IntQueue *q) {
    if (q == NULL) return 0;
    return q->size == q->capacity;
}

int intq_enqueue(IntQueue *q, int value) {
    if (q == NULL || q->buffer == NULL) return 0;
    if (intq_is_full(q)) return 0;
    q->buffer[q->tail] = value;
    q->tail = (q->tail + 1) % q->capacity;
    q->size += 1;
    return 1;
}

DequeueResult intq_dequeue(IntQueue *q) {
    DequeueResult res;
    res.success = 0;
    res.value = 0;
    if (q == NULL || q->buffer == NULL) return res;
    if (intq_is_empty(q)) return res;
    res.value = q->buffer[q->head];
    res.success = 1;
    q->head = (q->head + 1) % q->capacity;
    q->size -= 1;
    return res;
}

DequeueResult intq_peek(const IntQueue *q) {
    DequeueResult res;
    res.success = 0;
    res.value = 0;
    if (q == NULL || q->buffer == NULL) return res;
    if (intq_is_empty(q)) return res;
    res.value = q->buffer[q->head];
    res.success = 1;
    return res;
}

void intq_print(const IntQueue *q) {
    if (q == NULL || q->buffer == NULL) {
        printf("IntQueue{invalid}\n");
        return;
    }
    printf("IntQueue{size=%d, capacity=%d, data=[", q->size, q->capacity);
    for (int i = 0; i < q->size; i++) {
        int idx = (q->head + i) % q->capacity;
        printf("%d", q->buffer[idx]);
        if (i + 1 < q->size) printf(", ");
    }
    printf("]}\n");
}

static void test1(void) {
    printf("TEST 1: Basic enqueue/dequeue\n");
    IntQueue q;
    if (!intq_init(&q, 3)) {
        printf("Error: failed to init queue\n");
        return;
    }
    printf("enqueue 1 -> %s\n", intq_enqueue(&q, 1) ? "true" : "false");
    printf("enqueue 2 -> %s\n", intq_enqueue(&q, 2) ? "true" : "false");
    printf("enqueue 3 -> %s\n", intq_enqueue(&q, 3) ? "true" : "false");
    intq_print(&q);
    DequeueResult r = intq_peek(&q);
    if (r.success) printf("peek -> Result{success=true, value=%d}\n", r.value);
    else printf("peek -> Result{success=false}\n");
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue -> Result{success=false}\n");
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue -> Result{success=false}\n");
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue -> Result{success=false}\n");
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue (empty) -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue (empty) -> Result{success=false}\n");
    intq_print(&q);
    intq_free(&q);
}

static void test2(void) {
    printf("TEST 2: Overflow handling\n");
    IntQueue q;
    if (!intq_init(&q, 2)) {
        printf("Error: failed to init queue\n");
        return;
    }
    printf("enqueue 10 -> %s\n", intq_enqueue(&q, 10) ? "true" : "false");
    printf("enqueue 20 -> %s\n", intq_enqueue(&q, 20) ? "true" : "false");
    printf("enqueue 30 (should fail) -> %s\n", intq_enqueue(&q, 30) ? "true" : "false");
    DequeueResult r = intq_peek(&q);
    if (r.success) printf("peek -> Result{success=true, value=%d}\n", r.value);
    else printf("peek -> Result{success=false}\n");
    intq_print(&q);
    intq_free(&q);
}

static void test3(void) {
    printf("TEST 3: Peek on empty\n");
    IntQueue q;
    if (!intq_init(&q, 5)) {
        printf("Error: failed to init queue\n");
        return;
    }
    DequeueResult r = intq_peek(&q);
    if (r.success) printf("peek (empty) -> Result{success=true, value=%d}\n", r.value);
    else printf("peek (empty) -> Result{success=false}\n");
    printf("enqueue 7 -> %s\n", intq_enqueue(&q, 7) ? "true" : "false");
    r = intq_peek(&q);
    if (r.success) printf("peek -> Result{success=true, value=%d}\n", r.value);
    else printf("peek -> Result{success=false}\n");
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue -> Result{success=false}\n");
    r = intq_peek(&q);
    if (r.success) printf("peek (empty) -> Result{success=true, value=%d}\n", r.value);
    else printf("peek (empty) -> Result{success=false}\n");
    intq_free(&q);
}

static void test4(void) {
    printf("TEST 4: Wrap-around behavior\n");
    IntQueue q;
    if (!intq_init(&q, 3)) {
        printf("Error: failed to init queue\n");
        return;
    }
    printf("enqueue 1 -> %s\n", intq_enqueue(&q, 1) ? "true" : "false");
    printf("enqueue 2 -> %s\n", intq_enqueue(&q, 2) ? "true" : "false");
    DequeueResult r = intq_dequeue(&q);
    if (r.success) printf("dequeue -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue -> Result{success=false}\n");
    printf("enqueue 3 -> %s\n", intq_enqueue(&q, 3) ? "true" : "false");
    printf("enqueue 4 -> %s\n", intq_enqueue(&q, 4) ? "true" : "false");
    printf("isFull -> %s\n", intq_is_full(&q) ? "true" : "false");
    intq_print(&q);
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue -> Result{success=false}\n");
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue -> Result{success=false}\n");
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue -> Result{success=false}\n");
    printf("isEmpty -> %s\n", intq_is_empty(&q) ? "true" : "false");
    intq_free(&q);
}

static void test5(void) {
    printf("TEST 5: Capacity 1 edge case\n");
    IntQueue q;
    if (!intq_init(&q, 1)) {
        printf("Error: failed to init queue\n");
        return;
    }
    printf("enqueue 42 -> %s\n", intq_enqueue(&q, 42) ? "true" : "false");
    printf("enqueue 99 (should fail) -> %s\n", intq_enqueue(&q, 99) ? "true" : "false");
    DequeueResult r = intq_peek(&q);
    if (r.success) printf("peek -> Result{success=true, value=%d}\n", r.value);
    else printf("peek -> Result{success=false}\n");
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue -> Result{success=false}\n");
    r = intq_dequeue(&q);
    if (r.success) printf("dequeue (empty) -> Result{success=true, value=%d}\n", r.value);
    else printf("dequeue (empty) -> Result{success=false}\n");
    intq_free(&q);
}

int main(void) {
    test1();
    printf("\n");
    test2();
    printf("\n");
    test3();
    printf("\n");
    test4();
    printf("\n");
    test5();
    return 0;
}