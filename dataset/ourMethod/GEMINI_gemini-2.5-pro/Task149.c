#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
} Queue;

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (q == NULL) {
        perror("Failed to allocate memory for queue");
        return NULL;
    }
    q->front = NULL;
    q->rear = NULL;
    return q;
}

int isEmpty(Queue* q) {
    if (q == NULL) return 1;
    return q->front == NULL;
}

int enqueue(Queue* q, int data) {
    if (q == NULL) return 0;
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new node");
        return 0;
    }
    newNode->data = data;
    newNode->next = NULL;

    if (isEmpty(q)) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    return 1;
}

int dequeue(Queue* q, int* data) {
    if (q == NULL || data == NULL || isEmpty(q)) {
        return 0; 
    }
    Node* temp = q->front;
    *data = temp->data;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return 1;
}

int peek(Queue* q, int* data) {
    if (q == NULL || data == NULL || isEmpty(q)) {
        return 0;
    }
    *data = q->front->data;
    return 1;
}

void destroyQueue(Queue* q) {
    if (q == NULL) return;
    Node* current = q->front;
    Node* nextNode;
    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }
    free(q);
}

void run_tests() {
    printf("--- C Queue Test Cases ---\n");
    int value;

    // Test Case 1: Basic enqueue and dequeue
    printf("\n--- Test Case 1: Basic Operations ---\n");
    Queue* q1 = createQueue();
    if (!q1) return;
    enqueue(q1, 10);
    enqueue(q1, 20);
    if (dequeue(q1, &value)) printf("Dequeued: %d\n", value);
    if (peek(q1, &value)) printf("Peek: %d\n", value);
    if (dequeue(q1, &value)) printf("Dequeued: %d\n", value);
    printf("Is empty: %s\n", isEmpty(q1) ? "true" : "false");
    destroyQueue(q1);

    // Test Case 2: Dequeue from an empty queue
    printf("\n--- Test Case 2: Dequeue from Empty Queue ---\n");
    Queue* q2 = createQueue();
    if (!q2) return;
    if (!dequeue(q2, &value)) {
        printf("Dequeue from empty queue failed as expected.\n");
    }
    destroyQueue(q2);

    // Test Case 3: Peek at an empty queue
    printf("\n--- Test Case 3: Peek at Empty Queue ---\n");
    Queue* q3 = createQueue();
    if (!q3) return;
    if (!peek(q3, &value)) {
        printf("Peek at empty queue failed as expected.\n");
    }
    destroyQueue(q3);

    // Test Case 4: Enqueue after emptying the queue
    printf("\n--- Test Case 4: Enqueue after Emptying ---\n");
    Queue* q4 = createQueue();
    if (!q4) return;
    enqueue(q4, 1);
    enqueue(q4, 2);
    dequeue(q4, &value);
    dequeue(q4, &value);
    enqueue(q4, 3);
    if (peek(q4, &value)) printf("Peek: %d\n", value);
    printf("Is empty: %s\n", isEmpty(q4) ? "true" : "false");
    destroyQueue(q4);

    // Test Case 5: Multiple operations
    printf("\n--- Test Case 5: Multiple Operations ---\n");
    Queue* q5 = createQueue();
    if (!q5) return;
    enqueue(q5, 100);
    enqueue(q5, 200);
    enqueue(q5, 300);
    if (dequeue(q5, &value)) printf("Dequeued: %d\n", value);
    enqueue(q5, 400);
    if (peek(q5, &value)) printf("Peek: %d\n", value);
    if (dequeue(q5, &value)) printf("Dequeued: %d\n", value);
    if (dequeue(q5, &value)) printf("Dequeued: %d\n", value);
    if (peek(q5, &value)) printf("Peek: %d\n", value);
    destroyQueue(q5);
}

int main() {
    run_tests();
    return 0;
}