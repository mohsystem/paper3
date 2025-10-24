#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Node structure for the linked list
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Queue structure with front and rear pointers
typedef struct Queue {
    Node *front, *rear;
} Queue;

// Function to create a new node
Node* newNode(int k) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->data = k;
    temp->next = NULL;
    return temp;
}

// Function to create an empty queue
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

// Enqueue: Add an element to the end of the queue
void enqueue(Queue* q, int value) {
    Node* temp = newNode(value);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

// Dequeue: Remove and return the element from the front of the queue
int dequeue(Queue* q) {
    if (q->front == NULL) {
        fprintf(stderr, "Error: Queue is empty\n");
        return INT_MIN;
    }
    Node* temp = q->front;
    int data = temp->data;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return data;
}

// Peek: Return the element at the front of the queue without removing it
int peek(Queue* q) {
    if (q->front == NULL) {
        fprintf(stderr, "Error: Queue is empty\n");
        return INT_MIN;
    }
    return q->front->data;
}

// Main function with test cases
int main() {
    Queue* q = createQueue();

    printf("--- Test Case 1: Enqueue and Peek ---\n");
    enqueue(q, 10);
    enqueue(q, 20);
    enqueue(q, 30);
    printf("Peek: %d\n", peek(q)); // Expected: 10

    printf("\n--- Test Case 2: Dequeue and Peek ---\n");
    printf("Dequeued: %d\n", dequeue(q)); // Expected: 10
    printf("Peek: %d\n", peek(q)); // Expected: 20

    printf("\n--- Test Case 3: Dequeue multiple ---\n");
    printf("Dequeued: %d\n", dequeue(q)); // Expected: 20
    printf("Dequeued: %d\n", dequeue(q)); // Expected: 30

    printf("\n--- Test Case 4: Operations on empty queue ---\n");
    int peek_val = peek(q);
    if (peek_val == INT_MIN) {
        printf("Peek result on empty queue as expected.\n");
    }
    int dequeue_val = dequeue(q);
    if (dequeue_val == INT_MIN) {
        printf("Dequeue result on empty queue as expected.\n");
    }

    printf("\n--- Test Case 5: Enqueue after empty ---\n");
    enqueue(q, 40);
    enqueue(q, 50);
    printf("Peek: %d\n", peek(q)); // Expected: 40
    printf("Dequeued: %d\n", dequeue(q)); // Expected: 40
    printf("Peek: %d\n", peek(q)); // Expected: 50

    // Free remaining queue elements and the queue itself
    while (q->front != NULL) {
        dequeue(q);
    }
    free(q);

    return 0;
}