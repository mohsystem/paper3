#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int hasValue;
    int value;
} MaybeInt;

typedef struct {
    int *data;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
} IntQueue;

static size_t safe_grow_capacity(size_t current) {
    size_t max_cap = (size_t)1 << 30; // cap at about 1 billion entries
    if (current == 0) return 16;
    if (current >= max_cap) return max_cap;
    size_t proposed = current << 1;
    if (proposed < current || proposed > max_cap) {
        return max_cap;
    }
    return proposed;
}

static int reallocate_and_realign(IntQueue *q, size_t new_capacity) {
    if (new_capacity < q->size) return 0;
    int *new_data = (int*)calloc(new_capacity, sizeof(int));
    if (!new_data) return 0;

    if (q->size > 0) {
        if (q->head <= q->tail) {
            memcpy(new_data, q->data + q->head, q->size * sizeof(int));
        } else {
            size_t first_part = q->capacity - q->head;
            memcpy(new_data, q->data + q->head, first_part * sizeof(int));
            memcpy(new_data + first_part, q->data, q->tail * sizeof(int));
        }
    }

    free(q->data);
    q->data = new_data;
    q->capacity = new_capacity;
    q->head = 0;
    q->tail = q->size;
    return 1;
}

IntQueue* int_queue_create(size_t initial_capacity) {
    if (initial_capacity == 0) initial_capacity = 16;
    IntQueue *q = (IntQueue*)calloc(1, sizeof(IntQueue));
    if (!q) return NULL;
    q->data = (int*)calloc(initial_capacity, sizeof(int));
    if (!q->data) {
        free(q);
        return NULL;
    }
    q->capacity = initial_capacity;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    return q;
}

void int_queue_free(IntQueue *q) {
    if (!q) return;
    free(q->data);
    q->data = NULL;
    q->capacity = q->head = q->tail = q->size = 0;
    free(q);
}

int int_queue_enqueue(IntQueue *q, int value) {
    if (!q) return 0;
    if (q->size == q->capacity) {
        size_t new_capacity = safe_grow_capacity(q->capacity);
        if (new_capacity == q->capacity) return 0; // cannot grow
        if (!reallocate_and_realign(q, new_capacity)) return 0;
    }
    q->data[q->tail] = value;
    q->tail = (q->tail + 1) % q->capacity;
    q->size++;
    return 1;
}

MaybeInt int_queue_dequeue(IntQueue *q) {
    MaybeInt res = {0, 0};
    if (!q || q->size == 0) return res;
    res.hasValue = 1;
    res.value = q->data[q->head];
    q->data[q->head] = 0;
    q->head = (q->head + 1) % q->capacity;
    q->size--;
    return res;
}

MaybeInt int_queue_peek(const IntQueue *q) {
    MaybeInt res = {0, 0};
    if (!q || q->size == 0) return res;
    res.hasValue = 1;
    res.value = q->data[q->head];
    return res;
}

size_t int_queue_size(const IntQueue *q) {
    return q ? q->size : 0;
}

int int_queue_is_empty(const IntQueue *q) {
    return q ? (q->size == 0) : 1;
}

int main(void) {
    IntQueue *q = int_queue_create(16);
    if (!q) {
        fprintf(stderr, "Failed to create queue\n");
        return 1;
    }

    // Test 1: Enqueue and Peek
    int_queue_enqueue(q, 1);
    int_queue_enqueue(q, 2);
    int_queue_enqueue(q, 3);
    MaybeInt p1 = int_queue_peek(q);
    printf("Test1 Peek: %s\n", p1.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", p1.value), "")) : "null"); // print inline
    MaybeInt d1 = int_queue_dequeue(q);
    printf("\nTest1 Dequeue: %s\n", d1.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", d1.value), "")) : "null");

    // Test 2: Dequeue remaining and attempt extra dequeue
    MaybeInt d2 = int_queue_dequeue(q);
    printf("Test2 Dequeue: %s\n", d2.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", d2.value), "")) : "null");
    MaybeInt d3 = int_queue_dequeue(q);
    printf("Test2 Dequeue: %s\n", d3.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", d3.value), "")) : "null");
    MaybeInt d4 = int_queue_dequeue(q);
    printf("Test2 Dequeue Empty: %s\n", d4.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", d4.value), "")) : "null");

    // Test 3: Interleaved operations
    int_queue_enqueue(q, 10);
    MaybeInt p3a = int_queue_peek(q);
    printf("Test3 Peek: %s\n", p3a.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", p3a.value), "")) : "null");
    int_queue_enqueue(q, 20);
    MaybeInt d5 = int_queue_dequeue(q);
    printf("Test3 Dequeue: %s\n", d5.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", d5.value), "")) : "null");
    MaybeInt p3b = int_queue_peek(q);
    printf("Test3 Peek: %s\n", p3b.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", p3b.value), "")) : "null");

    // Test 4: Stress growth
    for (int i = 0; i < 100; ++i) int_queue_enqueue(q, i);
    printf("Test4 Size after 100 enqueues: %zu\n", int_queue_size(q));
    MaybeInt p4 = int_queue_peek(q);
    printf("Test4 Peek: %s\n", p4.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", p4.value), "")) : "null");
    for (int i = 0; i < 5; ++i) {
        MaybeInt dx = int_queue_dequeue(q);
        printf("Test4 Dequeue: %s\n", dx.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", dx.value), "")) : "null");
    }

    // Test 5: New queue, empty checks
    IntQueue *q2 = int_queue_create(16);
    if (!q2) {
        fprintf(stderr, "Failed to create queue q2\n");
        int_queue_free(q);
        return 1;
    }
    printf("Test5 IsEmpty: %s\n", int_queue_is_empty(q2) ? "true" : "false");
    MaybeInt p5 = int_queue_peek(q2);
    printf("Test5 Peek Empty: %s\n", p5.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", p5.value), "")) : "null");
    int_queue_enqueue(q2, 99);
    MaybeInt d6 = int_queue_dequeue(q2);
    printf("Test5 Dequeue: %s\n", d6.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", d6.value), "")) : "null");
    MaybeInt d7 = int_queue_dequeue(q2);
    printf("Test5 Dequeue Empty: %s\n", d7.hasValue ? (sprintf((char[32]){}, ""), (printf("%d", d7.value), "")) : "null");

    int_queue_free(q);
    int_queue_free(q2);
    return 0;
}