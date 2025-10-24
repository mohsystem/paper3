#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>

typedef struct Node {
    int value;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    size_t size;
} SafeQueue;

// Initialize queue
void init_queue(SafeQueue* q) {
    if (!q) return;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

// Free all nodes
void free_queue(SafeQueue* q) {
    if (!q) return;
    Node* curr = q->head;
    while (curr) {
        Node* next = curr->next;
        free(curr);
        curr = next;
    }
    q->head = q->tail = NULL;
    q->size = 0;
}

// Enqueue: returns 1 on success, 0 on failure
int enqueue(SafeQueue* q, int value) {
    if (!q) return 0;
    if (q->size == (size_t)-1) {
        return 0; // prevent overflow
    }
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return 0;
    n->value = value;
    n->next = NULL;
    if (!q->tail) {
        q->head = q->tail = n;
    } else {
        q->tail->next = n;
        q->tail = n;
    }
    q->size++;
    return 1;
}

// Dequeue: returns 1 on success and writes to out, 0 if empty or invalid args
int dequeue(SafeQueue* q, int* out) {
    if (!q || !out) return 0;
    if (!q->head) return 0;
    Node* n = q->head;
    *out = n->value;
    q->head = n->next;
    if (!q->head) q->tail = NULL;
    free(n);
    q->size--;
    return 1;
}

// Peek: returns 1 on success and writes to out, 0 if empty or invalid args
int peek(SafeQueue* q, int* out) {
    if (!q || !out) return 0;
    if (!q->head) return 0;
    *out = q->head->value;
    return 1;
}

size_t size_queue(SafeQueue* q) {
    if (!q) return 0;
    return q->size;
}

int main(void) {
    int out;

    // Test 1
    SafeQueue q1; init_queue(&q1);
    enqueue(&q1, 1);
    enqueue(&q1, 2);
    enqueue(&q1, 3);
    if (peek(&q1, &out)) printf("Test1 Peek (expect 1): %d\n", out); else printf("Test1 Peek (expect 1): null\n");
    if (dequeue(&q1, &out)) printf("Test1 Dequeue (expect 1): %d\n", out); else printf("Test1 Dequeue (expect 1): null\n");
    if (peek(&q1, &out)) printf("Test1 Peek (expect 2): %d\n", out); else printf("Test1 Peek (expect 2): null\n");
    printf("Test1 Size (expect 2): %zu\n", size_queue(&q1));
    free_queue(&q1);

    // Test 2
    SafeQueue q2; init_queue(&q2);
    if (dequeue(&q2, &out)) printf("Test2 Dequeue empty (expect null): %d\n", out); else printf("Test2 Dequeue empty (expect null): null\n");
    if (peek(&q2, &out)) printf("Test2 Peek empty (expect null): %d\n", out); else printf("Test2 Peek empty (expect null): null\n");
    printf("Test2 Size (expect 0): %zu\n", size_queue(&q2));
    free_queue(&q2);

    // Test 3
    SafeQueue q3; init_queue(&q3);
    enqueue(&q3, 10);
    if (dequeue(&q3, &out)) printf("Test3 Dequeue (expect 10): %d\n", out); else printf("Test3 Dequeue (expect 10): null\n");
    enqueue(&q3, 20);
    enqueue(&q3, 30);
    if (peek(&q3, &out)) printf("Test3 Peek (expect 20): %d\n", out); else printf("Test3 Peek (expect 20): null\n");
    printf("Test3 Size (expect 2): %zu\n", size_queue(&q3));
    free_queue(&q3);

    // Test 4
    SafeQueue q4; init_queue(&q4);
    enqueue(&q4, -5);
    enqueue(&q4, 0);
    if (dequeue(&q4, &out)) printf("Test4 Dequeue (expect -5): %d\n", out); else printf("Test4 Dequeue (expect -5): null\n");
    if (peek(&q4, &out)) printf("Test4 Peek (expect 0): %d\n", out); else printf("Test4 Peek (expect 0): null\n");
    printf("Test4 Size (expect 1): %zu\n", size_queue(&q4));
    free_queue(&q4);

    // Test 5
    SafeQueue q5; init_queue(&q5);
    for (int i = 100; i < 105; ++i) enqueue(&q5, i);
    if (dequeue(&q5, &out)) printf("Test5 Dequeue1 (expect 100): %d\n", out); else printf("Test5 Dequeue1 (expect 100): null\n");
    if (dequeue(&q5, &out)) printf("Test5 Dequeue2 (expect 101): %d\n", out); else printf("Test5 Dequeue2 (expect 101): null\n");
    if (dequeue(&q5, &out)) printf("Test5 Dequeue3 (expect 102): %d\n", out); else printf("Test5 Dequeue3 (expect 102): null\n");
    if (peek(&q5, &out)) printf("Test5 Peek (expect 103): %d\n", out); else printf("Test5 Peek (expect 103): null\n");
    printf("Test5 Size (expect 2): %zu\n", size_queue(&q5));
    free_queue(&q5);

    return 0;
}