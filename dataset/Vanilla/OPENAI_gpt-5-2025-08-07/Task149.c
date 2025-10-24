#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* data;
    int head;
    int tail;
    int size;
    int capacity;
} Queue;

Queue* createQueue(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->data = (int*)malloc(sizeof(int) * capacity);
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->capacity = capacity;
    return q;
}

void freeQueue(Queue* q) {
    if (q) {
        free(q->data);
        free(q);
    }
}

int enqueue(Queue* q, int x) {
    if (q->size == q->capacity) return 0; // fail
    q->data[q->tail] = x;
    q->tail = (q->tail + 1) % q->capacity;
    q->size++;
    return 1; // success
}

int dequeue(Queue* q, int* out) {
    if (q->size == 0) return 0; // fail
    *out = q->data[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;
    return 1; // success
}

int peek(Queue* q, int* out) {
    if (q->size == 0) return 0; // fail
    *out = q->data[q->head];
    return 1; // success
}

int main() {
    // Test 1: Basic enqueue, peek, and dequeue
    Queue* q1 = createQueue(5);
    enqueue(q1, 1);
    enqueue(q1, 2);
    enqueue(q1, 3);
    int val;
    if (peek(q1, &val)) printf("Test1 Peek: %d\n", val); else printf("Test1 Peek: null\n");
    if (dequeue(q1, &val)) printf("Test1 Dequeue: %d\n", val); else printf("Test1 Dequeue: null\n");
    if (peek(q1, &val)) printf("Test1 Peek after dequeue: %d\n", val); else printf("Test1 Peek after dequeue: null\n");
    freeQueue(q1);

    // Test 2: Peek on empty queue
    Queue* q2 = createQueue(3);
    if (peek(q2, &val)) printf("Test2 Peek on empty: %d\n", val); else printf("Test2 Peek on empty: null\n");
    freeQueue(q2);

    // Test 3: Dequeue on empty queue
    Queue* q3 = createQueue(2);
    if (dequeue(q3, &val)) printf("Test3 Dequeue on empty: %d\n", val); else printf("Test3 Dequeue on empty: null\n");
    freeQueue(q3);

    // Test 4: Fill to capacity and attempt extra enqueue
    Queue* q4 = createQueue(3);
    printf("Test4 Enqueue 1: %s\n", enqueue(q4, 1) ? "true" : "false");
    printf("Test4 Enqueue 2: %s\n", enqueue(q4, 2) ? "true" : "false");
    printf("Test4 Enqueue 3: %s\n", enqueue(q4, 3) ? "true" : "false");
    printf("Test4 Enqueue 4 (should fail): %s\n", enqueue(q4, 4) ? "true" : "false");
    freeQueue(q4);

    // Test 5: Wrap-around behavior
    Queue* q5 = createQueue(3);
    enqueue(q5, 10);
    enqueue(q5, 20);
    enqueue(q5, 30);
    if (dequeue(q5, &val)) printf("Test5 Dequeue: %d\n", val); else printf("Test5 Dequeue: null\n");
    if (dequeue(q5, &val)) printf("Test5 Dequeue: %d\n", val); else printf("Test5 Dequeue: null\n");
    printf("Test5 Enqueue 40: %s\n", enqueue(q5, 40) ? "true" : "false");
    printf("Test5 Enqueue 50: %s\n", enqueue(q5, 50) ? "true" : "false");
    if (dequeue(q5, &val)) printf("Test5 Dequeue: %d\n", val); else printf("Test5 Dequeue: null\n");
    if (dequeue(q5, &val)) printf("Test5 Dequeue: %d\n", val); else printf("Test5 Dequeue: null\n");
    if (dequeue(q5, &val)) printf("Test5 Dequeue: %d\n", val); else printf("Test5 Dequeue: null\n");
    if (dequeue(q5, &val)) printf("Test5 Dequeue on empty: %d\n", val); else printf("Test5 Dequeue on empty: null\n");
    freeQueue(q5);

    return 0;
}